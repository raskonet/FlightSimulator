#include "Terrain.h"
#include "Graphics.h"      // For GL calls via GLEW
#include "OpenGLUtils.h"   // For PRIMITIVE_RESTART_INDEX
#include <glm/gtc/type_ptr.hpp> // Potentially for matrix passing, though Shader class handles it
#include <iostream>        // For errors/debug

// Define texture parameters used by terrain textures
// Detail/Color map often repeats and uses mipmaps
const GLUtil::TextureParams terrainTexParams = {
    .texture_wrap = GL_REPEAT,
    .texture_mag_filter = GL_LINEAR,
    .texture_min_filter = GL_LINEAR_MIPMAP_LINEAR
};
// Height/Normal maps usually clamp and might not need mipmaps (linear is good)
const GLUtil::TextureParams heightmapTexParams = {
    .texture_wrap = GL_CLAMP_TO_EDGE,
    .texture_mag_filter = GL_LINEAR,
    .texture_min_filter = GL_LINEAR // Use simple linear filtering
};


Terrain::Terrain(int levels, int segments_per_block, float segment_size) :
    num_levels(std::max(1, levels)),
    block_segments(std::max(4, segments_per_block)),
    base_segment_size(std::max(0.1f, segment_size)),
    terrain_world_size(40000.0f), // Example: 40 km, adjust as needed

    // Initialize Geometry Blocks (using constructor of TerrainBlock/Seam)
    block_fine(block_segments, block_segments, base_segment_size, true), // Use primitive restart
    block_center(block_segments * 2 + 2, block_segments * 2 + 2, base_segment_size, true),
    block_col_fix(2, block_segments, base_segment_size, true),
    block_row_fix(block_segments, 2, base_segment_size, true),
    block_h_trim(block_segments * 2 + 2, 1, base_segment_size, true),
    block_v_trim(1, block_segments * 2 + 2, base_segment_size, true),
    block_seam(block_segments * 2 + 2, base_segment_size) // Seam uses DrawArrays
{
    // --- Load Shader ---
    shader = std::make_unique<Shader>("assets/shaders/terrain.vert", "assets/shaders/terrain.frag");
    if (!shader || shader->ID == 0) {
        throw std::runtime_error("Failed to load terrain shader.");
    }

    // --- Load Textures ---
    std::string heightPath = "assets/" + TERRAIN_DATA_PATH + "heightmap.png";
    std::string normalPath = "assets/" + TERRAIN_DATA_PATH + "normalmap.png";
    std::string detailPath = "assets/" + TERRAIN_DATA_PATH + "texture.png";

    // Load textures using the defined parameters
    Texture hm(heightPath.c_str(), heightmapTexParams);
    Texture nm(normalPath.c_str(), heightmapTexParams); // Use same params for normal map
    Texture dm(detailPath.c_str(), terrainTexParams);   // Use repeating params for detail map

    // Check validity immediately after construction attempt
    if (!hm.isValid()) {
        std::cerr << "Warning: Failed to load heightmap texture from: " << heightPath << std::endl;
    }
    if (!nm.isValid()) {
        std::cerr << "Warning: Failed to load normalmap texture from: " << normalPath << std::endl;
    }
    if (!dm.isValid()) {
        std::cerr << "Warning: Failed to load detail texture from: " << detailPath << std::endl;
    }
     // Consider throwing if heightmap is essential:
     // if (!hm.isValid()) { throw std::runtime_error("Essential heightmap texture failed to load."); }


    // Move loaded textures into member variables
    heightmap = std::move(hm);
    normalmap = std::move(nm);
    detailmap = std::move(dm);

    std::cout << "Terrain initialized." << std::endl;
    if (heightmap.isValid()) std::cout << "  Heightmap loaded (" << heightmap.Width << "x" << heightmap.Height << ")" << std::endl;
    if (normalmap.isValid()) std::cout << "  Normalmap loaded (" << normalmap.Width << "x" << normalmap.Height << ")" << std::endl;
    if (detailmap.isValid()) std::cout << "  Detailmap loaded (" << detailmap.Width << "x" << detailmap.Height << ")" << std::endl;
}

// Helper to calculate base offset for a level grid origin (using reference logic)
glm::vec2 Terrain::calculateLevelBaseOffset(int level, const glm::vec2& cameraPosXZ) const {
    float scale_l = std::pow(2.0f, static_cast<float>(level));
    // Snap grid size depends on next coarser level (L+1) according to reference idea
    float snap_scale = std::pow(2.0f, static_cast<float>(level + 1));
    float snapGridSize = base_segment_size * snap_scale;
    if (snapGridSize < 1e-6f) snapGridSize = 1.0f; // Avoid division by zero if scale gets too small

    glm::vec2 snappedPos = glm::floor(cameraPosXZ / snapGridSize) * snapGridSize;

    float currentBlockWorldSize = static_cast<float>(block_segments) * base_segment_size * scale_l;
    // Offset by 2 blocks relative to the snapped position (center of a 5x5 conceptual grid)
    glm::vec2 baseOffset = snappedPos - currentBlockWorldSize * 2.0f;
    return baseOffset;
}

// Helper to calculate model matrix for a block
glm::mat4 Terrain::calculateModelMatrix(const glm::vec2& positionXZ, float scale, float rotation_deg) const {
    glm::mat4 model = glm::mat4(1.0f);
    // Translate to the world position first
    model = glm::translate(model, glm::vec3(positionXZ.x, 0.0f, positionXZ.y));
    // Then rotate if needed
    if (std::abs(rotation_deg) > 1e-3f) {
        model = glm::rotate(model, glm::radians(rotation_deg), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    // Then scale the geometry (vertices are defined in local space relative to segment size)
    model = glm::scale(model, glm::vec3(scale));
    return model; // Order: Scale -> Rotate -> Translate (applied in reverse: T * R * S)
}


void Terrain::draw(const Camera& camera, const glm::mat4& projection, const glm::vec3& sunDirection) {
    if (!shader || !shader->ID) {
        std::cerr << "Terrain::draw error: Shader not valid!" << std::endl;
        return; // Cannot draw without shader
    }
     if (!heightmap.isValid()) {
        // std::cerr << "Terrain::draw error: Heightmap not valid!" << std::endl;
        // Maybe draw flat terrain if heightmap missing? For now, just return.
        return;
     }

    // --- OpenGL State ---
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(GLUtil::PRIMITIVE_RESTART_INDEX); // Use constant from Utils
    if (wireframe) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
    else { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

    // --- Activate Shader and Bind Textures ---
    shader->use();
    if (heightmap.isValid()) heightmap.bind(0); else return;
    if (normalmap.isValid()) normalmap.bind(1); else return; // Need normalmap now
    if (detailmap.isValid()) detailmap.bind(2); else return; // Need detailmap now

    // Set Uniforms
    shader->setMat4("u_View", camera.GetViewMatrix());
    shader->setMat4("u_Projection", projection);
    shader->setVec3("u_CameraPos", camera.Position);
    shader->setVec3("u_SunDirection", glm::normalize(sunDirection)); // <-- Set Sun Direction

    shader->setInt("u_Heightmap", 0);
    shader->setInt("u_Normalmap", 1); // <-- Set Normalmap unit
    shader->setInt("u_Texture", 2);   // <-- Set Detailmap unit

    shader->setFloat("u_TerrainSize", terrain_world_size);
    shader->setFloat("u_MaxHeight", 3000.0f);
    // shader->setVec3("u_Background", ...); // Background/fo    // --- Draw Clipmap Levels ---
    glm::vec3 cameraPos = camera.Position;
    glm::vec2 cameraPosXZ = glm::vec2(cameraPos.x, cameraPos.z);

    // Determine minimum level (higher camera -> coarser min level)
    int min_level = 0;
    min_level = static_cast<int>(glm::clamp(cameraPos.y / 3000.0f, 0.0f, (float)num_levels - 2.0f));

    for (int l = min_level; l < num_levels; ++l) {
        float scale = std::pow(2.0f, static_cast<float>(l));
        float scaled_segment_size = base_segment_size * scale;
        float block_world_size = static_cast<float>(block_segments) * scaled_segment_size;
        glm::vec2 base = calculateLevelBaseOffset(l, cameraPosXZ);

        // Set u_Model per block inside the loop, u_Scale not directly needed by shader if model matrix scales
        // shader->setFloat("u_Scale", scale); // Shader uses Model matrix now
        // shader->setFloat("u_Level", static_cast<float>(l)); // Shader only uses this for debug color in previous step

        // --- Draw Center (Finest Level Only) ---
        if (l == min_level) {
             // Reference uses a specific 'center' block geometry for L-shapes
             // Using block_center which has dimensions (2N+2) x (2N+2)
             glm::vec2 center_grid_origin = base + block_world_size * 1.5f; // Bottom-left of center 3x3 area
             // Need to position this block correctly relative to its geometry definition origin
             // If block geometry is centered, positionXZ is the world center.
             // If block geometry starts at (0,0), positionXZ is the world bottom-left.
             // Let's assume TerrainBlock centers its geometry.
             glm::vec2 center_block_world_pos = center_grid_origin + block_world_size; // Center of the 2x2 center area
             shader->setMat4("u_Model", calculateModelMatrix(center_block_world_pos, scale));
             block_center.draw(); // Draw the special center block

        } else {
            // --- Draw Trim/Fixup Geometry for Coarser Levels ---
            glm::vec2 prev_base = calculateLevelBaseOffset(l - 1, cameraPosXZ);
            glm::vec2 diff = glm::abs(base - prev_base); // Difference in base positions

            // Horizontal Trim (Position based on which row needs the trim)
            // If diff.y is large, means camera crossed into a new 'row' of coarse blocks vertically,
            // so the *opposite* edge needs the trim.
            glm::vec2 h_trim_pos;
            if (diff.y > block_world_size * 0.5f) { // Shifted vertically significantly
                // Draw trim along the top edge of the 3x3 inner area
                h_trim_pos = base + glm::vec2(block_world_size * 2.5f, block_world_size * 3.5f); // Centered on top edge
            } else {
                // Draw trim along the bottom edge of the 3x3 inner area
                h_trim_pos = base + glm::vec2(block_world_size * 2.5f, block_world_size * 1.5f); // Centered on bottom edge
            }
            shader->setMat4("u_Model", calculateModelMatrix(h_trim_pos, scale));
            block_h_trim.draw();

            // Vertical Trim (Position based on which column needs the trim)
            glm::vec2 v_trim_pos;
            if (diff.x > block_world_size * 0.5f) { // Shifted horizontally significantly
                // Draw trim along the right edge of the 3x3 inner area
                 v_trim_pos = base + glm::vec2(block_world_size * 3.5f, block_world_size * 2.5f); // Centered on right edge
            } else {
                // Draw trim along the left edge of the 3x3 inner area
                 v_trim_pos = base + glm::vec2(block_world_size * 1.5f, block_world_size * 2.5f); // Centered on left edge
            }
            shader->setMat4("u_Model", calculateModelMatrix(v_trim_pos, scale));
            block_v_trim.draw();
        }


        // --- Draw Outer Ring Blocks (5x5 grid, excluding inner 3x3) ---
        // This part requires the most refinement to match the reference's stitching
        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 5; ++c) {
                // Skip the inner 3x3 area - covered by center/trims/finer levels
                if (r >= 1 && r <= 3 && c >= 1 && c <= 3) continue;

                // Calculate block's bottom-left corner in world space
                glm::vec2 block_corner_pos = base + glm::vec2(c * block_world_size, r * block_world_size);
                 // Calculate center position assuming block geometry is centered
                glm::vec2 block_center_pos = block_corner_pos + block_world_size * 0.5f;

                // Set model matrix for this block
                shader->setMat4("u_Model", calculateModelMatrix(block_center_pos, scale));

                // Determine which block geometry to draw (tile, fixup, seam?)
                // Simplified: Use standard block_fine for all outer ring blocks
                block_fine.draw();

                // TODO: Add seam drawing logic here if needed, potentially rotating/positioning block_seam
                // Example: If on outer edge, draw a rotated seam
                // This needs careful calculation of seam position and rotation based on edge (N, S, E, W)
            }
        }
    } // End level loop

    // --- Restore OpenGL State ---
    if (wireframe) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }
    glDisable(GL_PRIMITIVE_RESTART);
    // Keep Culling enabled
    // glDisable(GL_CULL_FACE);

    // Unbind textures
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);

    shader->use(false); // Unbind shader
}

// Basic height sampling (Placeholder - Needs proper implementation)
float Terrain::getTerrainHeight(float worldX, float worldZ) {
    return 0.0f;
}

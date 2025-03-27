#ifndef TERRAIN_H
#define TERRAIN_H

#include "OpenGLUtils.h" // For GL types, wrappers, PRIMITIVE_RESTART_INDEX
#include "TerrainBlock.h" // For Block/Seam geometry
#include "Shader.h"     // Our Shader class
#include "Texture.h"    // Our Texture class
#include "Camera.h"     // Need camera info

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include <memory> // For unique_ptr
#include <cmath> // For std::pow, std::floor
#include <stdexcept> // <-- ADDED for error throwing in constructor

// Configurable path (relative to assets)
const std::string TERRAIN_DATA_PATH = "textures/terrain/default/"; // Example path

class Terrain {
public:
    bool wireframe = false; // Toggle wireframe rendering

    Terrain(int levels = 8, int segments_per_block = 16, float base_segment_size = 4.0f); // Constructor
    virtual ~Terrain() = default;

    // Main draw call
    void draw(const Camera& camera, const glm::mat4& projection, const glm::vec3& sunDirection); // Added projection

    // Get approximate terrain height at world XZ coordinates (optional, basic sampling)
    float getTerrainHeight(float worldX, float worldZ);

    float getTerrainSize() const { return terrain_world_size; }

private:
    // --- Configuration ---
    const int num_levels;
    const int block_segments;
    const float base_segment_size;
    const float terrain_world_size;
    // REMOVED: const unsigned int primitive_restart_index = 0xFFFF; // Use global one

    // --- OpenGL Resources ---
    std::unique_ptr<Shader> shader;
    Texture heightmap;
    Texture normalmap;
    Texture detailmap;

    // Geometry Blocks
    TerrainBlock block_fine;
    TerrainBlock block_center; // Consider if this specific geometry is needed vs just using block_fine
    TerrainBlock block_col_fix;
    TerrainBlock block_row_fix;
    TerrainBlock block_h_trim;
    TerrainBlock block_v_trim;
    TerrainSeam block_seam;

    // --- Helper Methods ---
    glm::vec2 calculateLevelBaseOffset(int level, const glm::vec2& cameraPosXZ) const;
    glm::mat4 calculateModelMatrix(const glm::vec2& positionXZ, float scale, float rotation_deg = 0.0f) const;
};

#endif // TERRAIN_H

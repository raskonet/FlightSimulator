#include "MiniMap.h"
#include "Graphics.h"       // Access shader, screen dimensions, OpenGL functions via glew.h
#include "Shader.h"         // <-- ***** ADD THIS LINE ***** For full Shader definition
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>         // For debugging if needed

// Ensure GL types like GLuint are visible (usually included via Graphics.h -> glew.h)
#include <GL/glew.h>

MiniMap::MiniMap() {
    // Initialize handles (good practice)
    VAO_quad = VBO_quad = EBO_quad = 0;
    VAO_tri = VBO_tri = 0;
    setupRenderData();
}

MiniMap::~MiniMap() {
    // Cleanup OpenGL resources
    if (VAO_quad != 0) glDeleteVertexArrays(1, &VAO_quad);
    if (VBO_quad != 0) glDeleteBuffers(1, &VBO_quad);
    if (EBO_quad != 0) glDeleteBuffers(1, &EBO_quad); // <-- Delete EBO_quad
    if (VAO_tri != 0) glDeleteVertexArrays(1, &VAO_tri);
    if (VBO_tri != 0) glDeleteBuffers(1, &VBO_tri);
    // Zero out handles after deletion (optional but good)
    VAO_quad = VBO_quad = EBO_quad = VAO_tri = VBO_tri = 0;
}

void MiniMap::setupRenderData() {
    // --- Simple Quad (for background) ---
    float quadVertices[] = {
        // Positions (centered at 0,0)
        -0.5f, -0.5f, // Bottom Left 0
         0.5f, -0.5f, // Bottom Right 1
         0.5f,  0.5f, // Top Right 2
        -0.5f,  0.5f  // Top Left 3
    };
    // Indices for the quad
    unsigned int quadIndices[] = {
        0, 1, 2, // First triangle
        0, 2, 3  // Second triangle
    };

    // Use member variables VAO_quad, VBO_quad, EBO_quad
    glGenVertexArrays(1, &VAO_quad);
    glGenBuffers(1, &VBO_quad);
    glGenBuffers(1, &EBO_quad); // Generate EBO for quad

    glBindVertexArray(VAO_quad); // Bind VAO first

    glBindBuffer(GL_ARRAY_BUFFER, VBO_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_quad); // Bind and fill EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // Vertex attribute pointer (only position for 2D minimap shader)
    // Location 0 corresponds to 'aPos' in minimap_shader.vert
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO target (VAO keeps track of VBO via attribute pointer)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Unbind VAO (EBO binding is stored within the VAO state)
    glBindVertexArray(0);
    // DO NOT unbind EBO_quad here (it's part of VAO state)


    // --- Simple Triangle (for aircraft direction marker) ---
     float triVertices[] = {
        // Positions (pointing up along +Y, centered at 0,0)
         0.0f,  0.5f, // Top point
        -0.3f, -0.5f, // Bottom left
         0.3f, -0.5f  // Bottom right
    };

    // Use member variables VAO_tri, VBO_tri
    glGenVertexArrays(1, &VAO_tri);
    glGenBuffers(1, &VBO_tri);

    glBindVertexArray(VAO_tri); // Bind triangle VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO_tri);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triVertices), triVertices, GL_STATIC_DRAW);

    // Vertex attribute pointer (Location 0, 2 floats, stride is 2*float)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VBO target
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Unbind triangle VAO
    glBindVertexArray(0);
}

// Render function - Now has access to Shader definition and member VAOs/VBOs
void MiniMap::render(const glm::vec3& aircraftPosition, const glm::quat& aircraftOrientation, float worldSize) {
    // Check required resources are valid
    if (!Graphics::minimapShader || VAO_quad == 0 || VAO_tri == 0) {
         // std::cerr << "Minimap render skipped: Missing shader or VAOs" << std::endl;
         return;
    }

    int screenWidth = Graphics::getWidth();
    int screenHeight = Graphics::getHeight();
    if (screenWidth <= 0 || screenHeight <= 0) return; // Avoid division by zero or invalid projection

    // --- Setup Orthographic Projection ---
    // Map screen coordinates (0,0) top-left to (W, H) bottom-right
    // Use 0.0f for near/far planes for simple 2D ortho
    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);

    // Use the minimap shader - Compiler now knows these functions
    Graphics::minimapShader->use();
    Graphics::minimapShader->setMat4("projection", projection);

    // --- State Changes for 2D Overlay ---
    GLboolean last_depth_test = glIsEnabled(GL_DEPTH_TEST); // Store previous depth test state
    glDisable(GL_DEPTH_TEST); // Disable depth testing for 2D overlay

    // --- Calculate MiniMap Screen Position & Size ---
    float mapDimScreen = screenHeight * miniMapSize; // Square minimap based on height
    float mapPosX = screenWidth - mapDimScreen - (screenWidth * padding);
    float mapPosY = screenHeight - mapDimScreen - (screenHeight * padding);
    glm::vec2 mapCenter = glm::vec2(mapPosX + mapDimScreen / 2.0f, mapPosY + mapDimScreen / 2.0f);

    // --- Render Background Quad ---
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(mapCenter, 0.0f)); // Translate to position
    model = glm::scale(model, glm::vec3(mapDimScreen, mapDimScreen, 1.0f)); // Scale to size

    Graphics::minimapShader->setMat4("model", model);
    Graphics::minimapShader->setVec4("objectColor", glm::vec4(0.2f, 0.2f, 0.2f, 0.7f)); // Semi-transparent dark grey

    glBindVertexArray(VAO_quad); // Bind quad VAO
    // Use glDrawElements because we setup EBO_quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // 6 indices for the quad
    // glBindVertexArray(0); // Keep VAO bound potentially if drawing more quads? No, unbind.


    // --- Render Aircraft Marker ---
    // Map world position (X, Z) to minimap coordinates relative to minimap center
    float scaleFactor = mapDimScreen / worldSize; // Screen pixels per world meter
    // Calculate offset from minimap center based on scaled world coords
    float markerOffsetX = aircraftPosition.x * scaleFactor;
    float markerOffsetY = aircraftPosition.z * scaleFactor; // Map world Z to minimap Y offset

    // Calculate absolute marker screen position
    float markerX = mapCenter.x + markerOffsetX;
    float markerY = mapCenter.y + markerOffsetY;

    // Clamp marker position to stay within the minimap background bounds
    markerX = glm::clamp(markerX, mapPosX + 2.0f, mapPosX + mapDimScreen - 2.0f); // Add small buffer
    markerY = glm::clamp(markerY, mapPosY + 2.0f, mapPosY + mapDimScreen - 2.0f);


    // Calculate aircraft heading angle (Yaw) from quaternion
    float aircraftYaw = glm::yaw(aircraftOrientation); // Radians around Y axis

    float markerSize = 10.0f; // Size of the triangle marker in pixels

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(markerX, markerY, 0.0f)); // Position marker
    // Rotate the marker. Our triangle points +Y. A yaw of 0 (forward = -Z world) should point DOWN on minimap.
    // Yaw increases CCW. We need to rotate CW on screen Z-axis, so use -aircraftYaw.
    model = glm::rotate(model, -aircraftYaw, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis for 2D screen
    model = glm::scale(model, glm::vec3(markerSize, markerSize, 1.0f)); // Scale marker size

    Graphics::minimapShader->setMat4("model", model);
    Graphics::minimapShader->setVec4("objectColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Bright Red marker

    glBindVertexArray(VAO_tri); // Bind triangle VAO
    // Draw the triangle using vertex data directly (no indices)
    glDrawArrays(GL_TRIANGLES, 0, 3); // 3 vertices form the triangle
    glBindVertexArray(0); // Unbind triangle VAO


    // --- Restore OpenGL State ---
    if (last_depth_test) {
        glEnable(GL_DEPTH_TEST); // Re-enable depth testing if it was enabled before
    }
}

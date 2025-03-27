#include "Map.h"
#include "Graphics.h"       // Access shader, screen dimensions, OpenGL functions via glew.h
#include "Texture.h"        // Include Texture header
#include "Shader.h"         // <-- ***** THE MISSING INCLUDE *****
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>         // For error messages

// Ensure GL types like GLuint are visible (usually included via Graphics.h -> glew.h)
#include <GL/glew.h>


Map::Map() {
    // Initialize member variables (good practice, although defaults are set in header)
    VAO = 0;
    VBO = 0;
    EBO = 0;

    setupGroundPlane(); // This should now correctly use member VAO, VBO, EBO

    // Load texture
    groundTexture = std::make_unique<Texture>("assets/terrain.png");
    if (!groundTexture || groundTexture->ID == 0) { // Check pointer too
         std::cerr << "Map Warning: Failed to load ground texture 'assets/terrain.png'." << std::endl;
         // The program can likely continue, just without a textured ground.
    }
}

Map::~Map() {
    // Cleanup OpenGL resources
    // Check handles before deleting
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO != 0) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
    // Texture unique_ptr handles its own cleanup (calling Texture destructor)
    groundTexture.reset();
}

float Map::getMapSize() const {
    return mapSize;
}


void Map::setupGroundPlane() {
    // Large quad vertices (Position, Texture Coords)
    float halfSize = mapSize / 2.0f;
    float texScale = 100.0f; // How many times texture repeats across map

    float vertices[] = {
        // Positions          // Texture Coords
        -halfSize, 0.0f, -halfSize,  0.0f,      0.0f,      // Bottom Left 0
         halfSize, 0.0f, -halfSize,  texScale,  0.0f,      // Bottom Right 1
         halfSize, 0.0f,  halfSize,  texScale,  texScale,  // Top Right 2
        -halfSize, 0.0f,  halfSize,  0.0f,      texScale   // Top Left 3
    };
    unsigned int indices[] = {
        0, 1, 2,  // First Triangle
        0, 2, 3   // Second Triangle
    };

    // Use the member variables VAO, VBO, EBO
    glGenVertexArrays(1, &VAO); // Assign to member VAO
    glGenBuffers(1, &VBO);      // Assign to member VBO
    glGenBuffers(1, &EBO);      // Assign to member EBO

    glBindVertexArray(VAO); // Bind the VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Bind VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Bind EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Configure vertex attributes
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VBO target (the VAO remembers the VBO itself)
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind VAO (this also implicitly unbinds the EBO from the context, but it's stored in the VAO state)
    glBindVertexArray(0);

    // DO NOT unbind EBO here - it's bound *to the VAO state*. Unbind VAO first.
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Redundant after unbinding VAO
}


// Render function - Now has access to Shader definition and member VAO
void Map::render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) {
    // Check pointers and handles are valid before proceeding
    if (!Graphics::basicShader || VAO == 0 || !groundTexture || groundTexture->ID == 0) {
        // Optionally print an error/warning only once if desired
        // std::cerr << "Map::render - Cannot render, missing shader, VAO, or texture." << std::endl;
        return;
    }

    // Now Graphics::basicShader points to a complete type
    Graphics::basicShader->use();

    // Bind texture to texture unit 0
    groundTexture->bind(GL_TEXTURE0);
    // Tell shader sampler "texture1" to use texture unit 0
    Graphics::basicShader->setInt("texture1", 0);

    // Model matrix (identity for the map, it's already at origin and scaled by vertex positions)
    glm::mat4 model = glm::mat4(1.0f);

    // Set shader uniforms - compiler now knows these functions
    Graphics::basicShader->setMat4("model", model);
    Graphics::basicShader->setMat4("view", view);
    Graphics::basicShader->setMat4("projection", projection);
    Graphics::basicShader->setBool("useTexture", true); // Use the ground texture
    Graphics::basicShader->setVec3("cameraPos", cameraPos); // Pass camera pos for fog

    // Draw the ground plane using the VAO and EBO
    glBindVertexArray(VAO); // Bind VAO (which includes EBO binding)
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // 6 indices for the two triangles
    glBindVertexArray(0); // Unbind VAO

    // Unbind texture unit (good practice)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

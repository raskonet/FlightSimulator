#ifndef TERRAIN_BLOCK_H
#define TERRAIN_BLOCK_H

#include "OpenGLUtils.h" // For VAO/VBO/EBO wrappers and PRIMITIVE_RESTART_INDEX
#include <glm/glm.hpp>
#include <vector>
#include <stdexcept> // For runtime_error

// Represents a single mesh block for the terrain clipmap
class TerrainBlock {
public:
    GLUtil::VertexArrayObject vao;
    GLUtil::VertexBuffer vbo;
    GLUtil::ElementBufferObject ebo;
    unsigned int index_count = 0;
    GLenum draw_mode = GL_TRIANGLE_STRIP;

    TerrainBlock(int width_segments, int height_segments, float segment_size, bool usePrimitiveRestart = true)
    {
        if (width_segments <= 0 || height_segments <= 0 || segment_size <= 0) {
            throw std::runtime_error("Invalid dimensions for TerrainBlock.");
        }

        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices; // Use unsigned int for indices matching GL_UNSIGNED_INT

        // Center the grid origin for potentially simpler model matrix later? Optional.
        float startX = -width_segments * segment_size * 0.5f;
        float startZ = -height_segments * segment_size * 0.5f;

        for (int y = 0; y <= height_segments; ++y) {
            for (int x = 0; x <= width_segments; ++x) {
                vertices.push_back({startX + x * segment_size, 0.0f, startZ + y * segment_size});
            }
        }

        if (usePrimitiveRestart) {
            draw_mode = GL_TRIANGLE_STRIP;
             // Use the global constant from OpenGLUtils.h
             // constexpr unsigned int primitive_restart_index = GLUtil::PRIMITIVE_RESTART_INDEX; // No need to redefine

            for (int y = 0; y < height_segments; ++y) {
                for (int x = 0; x <= width_segments; ++x) {
                    indices.push_back((y + 0) * (width_segments + 1) + x);
                    indices.push_back((y + 1) * (width_segments + 1) + x);
                }
                 // Add restart index using the global constant
                 if (y < height_segments - 1) {
                    indices.push_back(GLUtil::PRIMITIVE_RESTART_INDEX);
                 }
            }
            index_count = static_cast<unsigned int>(indices.size());
        } else {
             draw_mode = GL_TRIANGLES;
             for (int y = 0; y < height_segments; ++y) {
                 for (int x = 0; x < width_segments; ++x) {
                     unsigned int tl = (y + 0) * (width_segments + 1) + x;
                     unsigned int tr = tl + 1;
                     unsigned int bl = (y + 1) * (width_segments + 1) + x;
                     unsigned int br = bl + 1;
                     indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
                     indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
                 }
             }
             index_count = static_cast<unsigned int>(indices.size());
        }

        if (vertices.empty() || indices.empty()) {
             throw std::runtime_error("Failed to generate geometry for TerrainBlock.");
        }

        vao.bind();
        vbo.buffer(vertices);
        ebo.buffer(indices);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);
        vao.unbind();
    }

    void draw() const {
        if (index_count == 0) return;
        vao.bind();
        glDrawElements(draw_mode, index_count, GL_UNSIGNED_INT, 0);
        vao.unbind();
    }
};

// Simple geometry for Seam
class TerrainSeam {
public:
    GLUtil::VertexArrayObject vao;
    GLUtil::VertexBuffer vbo;
    unsigned int vertex_count = 0;

    TerrainSeam(int columns, float segment_size) {
         if (columns <= 0 || segment_size <= 0) {
             throw std::runtime_error("Invalid dimensions for TerrainSeam.");
         }

         std::vector<glm::vec3> vertices;
         // Center the seam geometry?
         float totalWidth = columns * segment_size * 2.0f;
         float startX = -totalWidth / 2.0f;

         for (int x = 0; x < columns; ++x) {
             float x_start = startX + x * segment_size * 2.0f;
             float x_mid = x_start + segment_size;
             float x_end = x_start + segment_size * 2.0f;
             vertices.push_back({x_start, 0.0f, 0.0f}); // Base-left
             vertices.push_back({x_mid,   0.0f, 0.0f}); // Top-mid (Y set by shader)
             vertices.push_back({x_end,   0.0f, 0.0f}); // Base-right
         }
         vertex_count = static_cast<unsigned int>(vertices.size());

         if (vertices.empty()){
              throw std::runtime_error("Failed to generate geometry for TerrainSeam.");
         }

         vao.bind();
         vbo.buffer(vertices);
         glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
         glEnableVertexAttribArray(0);
         vao.unbind();
    }

     void draw() const {
         if (vertex_count == 0) return;
         vao.bind();
         glDrawArrays(GL_TRIANGLES, 0, vertex_count);
         vao.unbind();
     }
};

#endif // TERRAIN_BLOCK_H

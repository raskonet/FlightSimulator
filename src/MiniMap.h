#ifndef MINIMAP_H
#define MINIMAP_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GL/glew.h>

class MiniMap {
public:
    MiniMap();
    ~MiniMap();

    // Renders the minimap in a corner of the screen
    void render(const glm::vec3& aircraftPosition, const glm::quat& aircraftOrientation, float worldSize);

private:
    GLuint VAO_quad = 0, VBO_quad = 0,EBO_quad=0; // For background/aircraft marker
    GLuint VAO_tri = 0, VBO_tri = 0; // For aircraft direction triangle

    float miniMapSize = 0.25f; // Fraction of screen height
    float padding = 0.02f;    // Padding from screen edge

    void setupRenderData();
};

#endif // MINIMAP_H

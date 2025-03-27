#ifndef MAP_H
#define MAP_H

#include <glm/glm.hpp>
#include <memory> // For unique_ptr
#include <GL/glew.h> 
// Forward declarations
class Shader;
class Texture;

class Map {
public:
    Map();
    ~Map();

       // ...
    // void render(const glm::mat4& view, const glm::mat4& projection); // OLD
    void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos); // NEW
    // ...
    float getMapSize() const; // <-- ADDED GETTER DECLARATION

private:
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;// Added EBO member for proper cleanup
    std::unique_ptr<Texture> groundTexture;
    float mapSize = 5000.0f; // Size of the ground plane

    void setupGroundPlane();
};

#endif // MAP_H

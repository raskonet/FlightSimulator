#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory> // For unique_ptr

// Forward declarations
class Shader;

#include <GL/glew.h>

class Aircraft {
public:
    // State
    glm::vec3 position;
    glm::quat orientation;
    glm::vec3 velocity;
    glm::vec3 angularVelocity; // Radians per second around local axes

    // Properties
    float mass; // kg

    // Control Inputs (set externally)
    float throttleInput = 0.0f; // 0.0 to 1.0
    float pitchInput = 0.0f;    // -1.0 to 1.0
    float rollInput = 0.0f;     // -1.0 to 1.0
    float yawInput = 0.0f;      // -1.0 to 1.0
Aircraft(glm::vec3 startPos = glm::vec3(0.0f, 100.0f, 0.0f), float startMass = 1500.0f);
    ~Aircraft();

    void update(float deltaTime);
    void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos); // Takes cameraPos
  //getters  
  glm::vec3 getPosition() const;
    glm::quat getOrientation() const;
    glm::vec3 getVelocity() const;
    float getSpeed() const; // km/h
    float getAltitude() const; // meters

private:
    // Rendering resources
    GLuint VAO = 0, VBO = 0, EBO =0;
    void setupModel(); // Sets up the VAO/VBO for the placeholder model

    // Physics calculations
    glm::vec3 calculateAerodynamicForces(const glm::vec3& currentVelocity, const glm::quat& currentOrientation);
    glm::vec3 calculateThrustForce(const glm::quat& currentOrientation);
    glm::vec3 calculateControlTorques(const glm::quat& currentOrientation);
};

#endif // AIRCRAFT_H

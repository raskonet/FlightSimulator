#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Camera options
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 GetViewMatrix() const;

    // Follow target (e.g., aircraft)
    void Follow(const glm::vec3& targetPosition, const glm::quat& targetOrientation, float distance, float heightOffset);

private:
    void updateCameraVectors(const glm::quat& targetOrientation);
};

#endif // CAMERA_H

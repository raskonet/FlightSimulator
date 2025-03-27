#include "Camera.h"
#include <glm/gtc/quaternion.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 up) :
    Position(position),
    Front(glm::vec3(0.0f, 0.0f, -1.0f)), // Default orientation
    WorldUp(up),
    Zoom(45.0f) // Default zoom
{
    // Initial calculation - will be overwritten by Follow
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::Follow(const glm::vec3& targetPosition, const glm::quat& targetOrientation, float distance, float heightOffset) {
    updateCameraVectors(targetOrientation); // Update orientation first based on aircraft

    // Calculate camera position: behind, slightly above the target
    glm::vec3 offset = Front * -distance + Up * heightOffset; // Go backward and upward relative to aircraft orientation
    Position = targetPosition + offset;
}


void Camera::updateCameraVectors(const glm::quat& targetOrientation) {
    // Calculate the new Front, Right and Up vectors based on the aircraft's orientation
    Front = glm::normalize(targetOrientation * glm::vec3(0.0f, 0.0f, -1.0f)); // Aircraft's forward direction
    Right = glm::normalize(targetOrientation * glm::vec3(1.0f, 0.0f, 0.0f)); // Aircraft's right direction
    // Up = glm::normalize(glm::cross(Right, Front)); // Recalculate Up based on new Front and Right
    Up = glm::normalize(targetOrientation * glm::vec3(0.0f, 1.0f, 0.0f)); // Aircraft's up direction
}

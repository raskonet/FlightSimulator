// PhysicsEngine.cpp
#include "PhysicsEngine.h"

bool PhysicsEngine::checkCollision(const glm::vec3& position, float radius) {
    // Simple ground collision check
    return position.y - radius <= 0.0f;
}

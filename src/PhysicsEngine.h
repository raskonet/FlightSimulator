// PhysicsEngine.h
#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include <glm/glm.hpp>

class PhysicsEngine {
public:
    static bool checkCollision(const glm::vec3& position, float radius);
};

#endif // PHYSICSENGINE_H

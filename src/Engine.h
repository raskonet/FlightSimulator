#ifndef ENGINE_H
#define ENGINE_H

#include "RigidBody.h" // Needs RigidBody to apply force
#include "PhysicsConfig.h" // For BODY_FORWARD

class Engine {
public:
    float max_thrust;  // Max thrust force in Newtons
    float throttle;    // Current throttle setting (0.0 to 1.0)

    Engine(float maxThrust = PhysicsConfig::DEFAULT_THRUST) :
        max_thrust(maxThrust),
        throttle(0.0f) // Start with engine off
    {}

    void setThrottle(float t) {
        throttle = glm::clamp(t, 0.0f, 1.0f);
    }

    // Applies thrust force to the rigid body
    void applyForce(RigidBody* rigid_body) const {
        if (!rigid_body || throttle <= 0.0f) return;

        // Calculate current thrust force
        float current_thrust = throttle * max_thrust;

        // Apply force along the body's forward axis (+X assumed here, adjust if needed)
        // Force is applied at CG, so use addForceBody (no torque generated)
        rigid_body->addForceBody(PhysicsConfig::BODY_FORWARD * current_thrust);
    }
};

#endif // ENGINE_H

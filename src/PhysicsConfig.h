#ifndef PHYSICS_CONFIG_H
#define PHYSICS_CONFIG_H

#include <glm/glm.hpp>

namespace PhysicsConfig {
    const float GRAVITY = 9.81f;
    const float AIR_DENSITY = 1.225f; // kg/m^3 at sea level

    // Aircraft specific (Example values - tune these!)
    const float WING_AREA = 20.0f; // m^2
    const float LIFT_COEFFICIENT_SLOPE = 0.1f; // Per degree Angle of Attack (Simplified)
    const float MAX_LIFT_COEFFICIENT = 1.5f;
    const float DRAG_COEFFICIENT_BASE = 0.04f; // Base drag
    const float DRAG_INDUCED_FACTOR = 0.05f;   // Drag due to lift

    const float MAX_THRUST = 25000.0f; // Newtons
    const float PITCH_TORQUE_FACTOR = 5000.0f;
    const float ROLL_TORQUE_FACTOR = 4000.0f;
    const float YAW_TORQUE_FACTOR = 3000.0f;

    const float ANGULAR_DAMPING = 0.8f; // Dampen rotational speed
}

#endif // PHYSICS_CONFIG_H

#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class RigidBody {
protected: // Protected so derived classes can access directly if needed
    glm::vec3 m_force_accumulator_world; // Force accumulator in world space
    glm::vec3 m_torque_accumulator_body; // Torque accumulator in body space

public:
    // --- Static Properties ---
    float mass = 1.0f;                  // Mass (kg)
    glm::mat3 inertia_tensor_body{};    // Inertia tensor in body space (kg*m^2)
    glm::mat3 inv_inertia_tensor_body{};// Inverse of the inertia tensor in body space

    // --- State Variables ---
    glm::vec3 position_world;           // Position in world space (m)
    glm::quat orientation_world;        // Orientation in world space (unit quaternion)
    glm::vec3 velocity_world;           // Linear velocity in world space (m/s)
    glm::vec3 angular_velocity_body;    // Angular velocity in body space (rad/s)

    // --- Control ---
    bool apply_gravity = true;

    // --- Constructor ---
    RigidBody(); // Default constructor
    virtual ~RigidBody() = default; // Virtual destructor for inheritance

    // --- Configuration ---
    void setInertiaTensor(const glm::mat3& inertia_body);

    // --- Coordinate Transformations ---
    // Transform direction from body space to world space
    glm::vec3 bodyToWorldDir(const glm::vec3& dir_body) const;
    // Transform direction from world space to body space
    glm::vec3 worldToBodyDir(const glm::vec3& dir_world) const;
    // Transform point from body space to world space
    glm::vec3 bodyToWorldPoint(const glm::vec3& point_body) const;
    // Transform point from world space to body space
    glm::vec3 worldToBodyPoint(const glm::vec3& point_world) const;

    // --- Velocity Calculation ---
    // Get world-space velocity of a point offset from the CG (offset given in body space)
    glm::vec3 getPointVelocityWorld(const glm::vec3& point_body) const;

    // --- Force Application ---
    // Apply a force specified in world space at the center of gravity
    void addForceWorld(const glm::vec3& force_world);
    // Apply a force specified in body space at the center of gravity
    void addForceBody(const glm::vec3& force_body);
    // Apply a force specified in body space at a point offset in body space
    void addForceAtPointBody(const glm::vec3& force_body, const glm::vec3& point_body);
     // Apply a force specified in world space at a point offset in world space (less common for aero)
    void addForceAtPointWorld(const glm::vec3& force_world, const glm::vec3& point_world);

    // --- Simulation Update ---
    // Integrates physics state forward by dt seconds
    virtual void update(float dt);

    // --- Reset Accumulators (called internally by update) ---
    void clearAccumulators();
};

#endif // RIGIDBODY_H

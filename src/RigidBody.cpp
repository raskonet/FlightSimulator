#include "RigidBody.h"
#include "PhysicsConfig.h" // For GRAVITY
#include <glm/gtc/matrix_inverse.hpp> // For matrix inverse

RigidBody::RigidBody() :
    mass(1.0f),
    inertia_tensor_body(1.0f), // Identity matrix initially
    inv_inertia_tensor_body(1.0f),
    position_world(0.0f),
    orientation_world(1.0f, 0.0f, 0.0f, 0.0f), // Identity quaternion
    velocity_world(0.0f),
    angular_velocity_body(0.0f),
    m_force_accumulator_world(0.0f),
    m_torque_accumulator_body(0.0f),
    apply_gravity(true)
{}

void RigidBody::setInertiaTensor(const glm::mat3& inertia_body) {
    inertia_tensor_body = inertia_body;
    // Calculate and store the inverse. Use affineInverse for potentially non-orthogonal matrices if needed,
    // but inertia tensors are symmetric, so inverse should be fine. Handle singularity if necessary.
    inv_inertia_tensor_body = glm::inverse(inertia_tensor_body);
    // TODO: Add check for near-zero determinant (singularity) if needed
}

glm::vec3 RigidBody::bodyToWorldDir(const glm::vec3& dir_body) const {
    return orientation_world * dir_body;
}

glm::vec3 RigidBody::worldToBodyDir(const glm::vec3& dir_world) const {
    return glm::inverse(orientation_world) * dir_world;
}

glm::vec3 RigidBody::bodyToWorldPoint(const glm::vec3& point_body) const {
     return position_world + (orientation_world * point_body);
}

glm::vec3 RigidBody::worldToBodyPoint(const glm::vec3& point_world) const {
     return glm::inverse(orientation_world) * (point_world - position_world);
}


glm::vec3 RigidBody::getPointVelocityWorld(const glm::vec3& point_body) const {
    // Velocity of point = translational velocity + rotational velocity
    // Rotational velocity (world) = cross(angular_velocity_world, radius_vector_world)
    // angular_velocity_world = bodyToWorldDir(angular_velocity_body)
    // radius_vector_world = bodyToWorldDir(point_body)
    glm::vec3 angular_velocity_world = bodyToWorldDir(angular_velocity_body);
    glm::vec3 radius_vector_world = bodyToWorldDir(point_body);
    return velocity_world + glm::cross(angular_velocity_world, radius_vector_world);
}

void RigidBody::addForceWorld(const glm::vec3& force_world) {
    m_force_accumulator_world += force_world;
}

void RigidBody::addForceBody(const glm::vec3& force_body) {
    m_force_accumulator_world += bodyToWorldDir(force_body);
}

void RigidBody::addForceAtPointBody(const glm::vec3& force_body, const glm::vec3& point_body) {
    // Apply force to linear motion accumulator
    glm::vec3 force_world = bodyToWorldDir(force_body);
    m_force_accumulator_world += force_world;

    // Apply torque to angular motion accumulator
    // Torque (body space) = cross(point_body, force_body)
    m_torque_accumulator_body += glm::cross(point_body, force_body);
}

void RigidBody::addForceAtPointWorld(const glm::vec3& force_world, const glm::vec3& point_world) {
     // Apply force to linear motion accumulator
     m_force_accumulator_world += force_world;

     // Calculate point relative to CG in world space
     glm::vec3 point_relative_world = point_world - position_world;
     // Calculate torque in world space: T_world = cross(point_relative_world, force_world)
     glm::vec3 torque_world = glm::cross(point_relative_world, force_world);
     // Convert torque to body space and accumulate
     m_torque_accumulator_body += worldToBodyDir(torque_world);
}

void RigidBody::clearAccumulators() {
    m_force_accumulator_world = glm::vec3(0.0f);
    m_torque_accumulator_body = glm::vec3(0.0f);
}

void RigidBody::update(float dt) {
    if (mass <= 0.0f || dt <= 0.0f) return; // Safety check

    // --- Linear Motion ---
    // Add gravity if applicable
    glm::vec3 final_force = m_force_accumulator_world;
    if (apply_gravity) {
        final_force += glm::vec3(0.0f, -PhysicsConfig::GRAVITY * mass, 0.0f); // Assuming world Y is up
    }

    // Calculate linear acceleration (world space)
    glm::vec3 linear_acceleration = final_force / mass;

    // Integrate velocity (world space) - Using semi-implicit Euler
    velocity_world += linear_acceleration * dt;

    // Integrate position (world space)
    position_world += velocity_world * dt;


    // --- Angular Motion ---
    // Calculate angular acceleration (body space)
    // alpha = I_inv * (Torque - w x (I * w))
    // where w is angular_velocity_body, I is inertia_tensor_body, Torque is m_torque_accumulator_body
    glm::vec3 Iw = inertia_tensor_body * angular_velocity_body;
    glm::vec3 gyro_term = glm::cross(angular_velocity_body, Iw); // Gyroscopic precession term
    glm::vec3 net_torque_body = m_torque_accumulator_body - gyro_term;

    glm::vec3 angular_acceleration_body = inv_inertia_tensor_body * net_torque_body;

    // Integrate angular velocity (body space)
    angular_velocity_body += angular_acceleration_body * dt;

    // Integrate orientation (world space)
    // Create a delta quaternion from angular velocity
    // dQ/dt = 0.5 * Q * PureQuaternion(angular_velocity_body)
    // Approximate update: Q_new = Q_old + dt * 0.5 * Q_old * PureQuat(w_body)
    glm::quat w_quat_body(0.0f, angular_velocity_body.x, angular_velocity_body.y, angular_velocity_body.z);
    orientation_world += (orientation_world * w_quat_body) * (0.5f * dt);

    // Re-normalize orientation quaternion to prevent drift due to numerical errors
    orientation_world = glm::normalize(orientation_world);


    // --- Reset Accumulators for next frame ---
    clearAccumulators();
}

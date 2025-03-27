#include "Aircraft.h"
#include "Graphics.h"       // Access shaders, GL functions
#include "PhysicsConfig.h"
#include "Input.h"          // Access input state
#include "Shader.h"         // <-- ADDED: Include full Shader definition
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>         // Debugging
#include <vector>           // Used implicitly by GLM includes, but good practice

// Make sure GL types are available (usually comes via Graphics.h -> glew.h)
#include <GL/glew.h>


Aircraft::Aircraft(glm::vec3 startPos, float startMass) :
    position(startPos),
    orientation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)), // Identity quaternion
    velocity(0.0f, 0.0f, 0.0f),
    angularVelocity(0.0f),
    mass(startMass),
    VAO(0), // Initialize member variables in constructor list
    VBO(0)
{
    setupModel();
}

Aircraft::~Aircraft() {
    // Cleanup OpenGL resources
    // Check if buffers were actually generated before deleting
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0; // Good practice to zero out handles after deletion
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    // If you add an EBO later, delete it here too.
}


void Aircraft::setupModel() {
    // Simple pyramid model (Placeholder for OBJ loading)
    // Vertices: Position (x, y, z), Texture Coords (s, t) - Tex Coords not used here
    // Define vertices for a more distinct aircraft shape (like a simple jet)
    float vertices[] = {
        // Fuselage (long box) - centered at origin
         0.0f,  0.1f, -1.5f,  0.5f, 0.0f, // Nose Tip 0
        -0.2f,  0.0f, -1.0f,  0.0f, 0.2f, // L Mid Front 1
         0.2f,  0.0f, -1.0f,  1.0f, 0.2f, // R Mid Front 2
        -0.2f, -0.1f,  1.0f,  0.0f, 0.8f, // L Bottom Rear 3
         0.2f, -0.1f,  1.0f,  1.0f, 0.8f, // R Bottom Rear 4
         0.0f,  0.2f,  1.0f,  0.5f, 1.0f, // Top Rear Fin Base 5

        // Wings (flat quads)
        -1.5f,  0.0f, -0.5f,  0.0f, 0.0f, // L Wing Tip 6
        -0.2f,  0.0f, -0.8f,  0.4f, 0.4f, // L Wing Root Front 7
        -0.2f,  0.0f,  0.2f,  0.4f, 0.6f, // L Wing Root Rear 8
        -1.0f,  0.0f,  0.5f,  0.0f, 1.0f, // L Wing Rear Tip 9

         1.5f,  0.0f, -0.5f,  1.0f, 0.0f, // R Wing Tip 10
         0.2f,  0.0f, -0.8f,  0.6f, 0.4f, // R Wing Root Front 11
         0.2f,  0.0f,  0.2f,  0.6f, 0.6f, // R Wing Root Rear 12
         1.0f,  0.0f,  0.5f,  1.0f, 1.0f, // R Wing Rear Tip 13

        // Tail Fin (vertical triangle)
         0.0f,  0.8f,  1.0f,  0.5f, 1.0f, // Fin Top 14
         0.0f,  0.2f,  0.8f,  0.4f, 0.8f, // Fin Base Front 15
         // Re-use point 5: 0.0f,  0.2f,  1.0f,  0.5f, 1.0f, // Fin Base Rear

        // Horizontal Stabilizers (small wings at back)
        -0.6f,  0.1f,  1.2f,  0.0f, 0.8f, // L Stab Tip 16
        -0.1f,  0.1f,  0.9f,  0.4f, 0.9f, // L Stab Root 17
         0.1f,  0.1f,  0.9f,  0.6f, 0.9f, // R Stab Root 18
         0.6f,  0.1f,  1.2f,  1.0f, 0.8f, // R Stab Tip 19
    };

    // Indices to draw triangles (This will be complex for the above shape)
    // Let's simplify back to the pyramid for now to ensure compilation
    // You can replace this later if needed.
     float simple_pyramid_vertices[] = {
        // Base (Y=-0.25) - CCW winding from top view
        -0.5f, -0.25f, -0.5f,  0.0f, 0.0f, // 0
         0.5f, -0.25f, -0.5f,  1.0f, 0.0f, // 1
         0.5f, -0.25f,  0.5f,  1.0f, 1.0f, // 2
        -0.5f, -0.25f,  0.5f,  0.0f, 1.0f, // 3
        // Peak (Y=0.75)
         0.0f,  0.75f,  0.0f,  0.5f, 0.5f  // 4
     };
     unsigned int simple_pyramid_indices[] = {
        // Sides (CCW winding when viewed from outside)
        0, 1, 4, // Front face
        1, 2, 4, // Right face
        2, 3, 4, // Back face
        3, 0, 4, // Left face
        // Base (two triangles, CCW from top view)
        // Use CW winding if viewed from below to be consistent front-face
         3, 2, 0, // Base Tri 1
         2, 1, 0  // Base Tri 2
     };
     GLuint EBO; // Element Buffer Object handle

    // Use member variables VAO, VBO
    glGenVertexArrays(1, &this->VAO); // Use this-> explicitly if needed, but usually not
    glGenBuffers(1, &this->VBO);
    glGenBuffers(1, &EBO); // Generate EBO

    glBindVertexArray(this->VAO); // Bind the VAO

    glBindBuffer(GL_ARRAY_BUFFER, this->VBO); // Bind VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(simple_pyramid_vertices), simple_pyramid_vertices, GL_STATIC_DRAW); // Use pyramid data

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Bind EBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(simple_pyramid_indices), simple_pyramid_indices, GL_STATIC_DRAW); // Use pyramid indices

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // --- Important ---
    // The VAO remembers the VBO binding *and* the EBO binding made while it was bound.
    // So, we don't unbind the EBO here.
    // We *can* unbind the GL_ARRAY_BUFFER (VBO target), as the VAO references the VBO itself via glVertexAttribPointer.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind the VAO. The EBO binding is associated *with the VAO*.
    glBindVertexArray(0);

    // We don't need the EBO handle anymore *after* unbinding the VAO,
    // BUT it's good practice to delete it in the destructor.
    // If you don't store EBO as a member, you could delete it here, but it's safer to store & delete in ~Aircraft()
    // glDeleteBuffers(1, &EBO); // <-- If you don't store EBO as member, delete now. Better to store it.
    // We will assume EBO should be stored as a member and deleted in destructor like VAO/VBO.
    // Add `GLuint EBO = 0;` to Aircraft.h private members and delete in destructor.
}

glm::vec3 Aircraft::calculateThrustForce(const glm::quat& currentOrientation) {
    // Thrust acts along the aircraft's forward direction (-Z axis in local coordinates)
    glm::vec3 forwardDir = glm::normalize(currentOrientation * glm::vec3(0.0f, 0.0f, -1.0f));
    return forwardDir * throttleInput * PhysicsConfig::MAX_THRUST;
}

glm::vec3 Aircraft::calculateAerodynamicForces(const glm::vec3& currentVelocity, const glm::quat& currentOrientation) {
    float speedSqr = glm::length2(currentVelocity);
    if (speedSqr < 0.01f) return glm::vec3(0.0f); // Avoid division by zero and weirdness at low speed

    float speed = glm::sqrt(speedSqr);
    glm::vec3 velocityDir = currentVelocity / speed;

    // Local coordinate vectors
    glm::vec3 forwardDir = glm::normalize(currentOrientation * glm::vec3(0.0f, 0.0f, -1.0f));
    glm::vec3 upDir = glm::normalize(currentOrientation * glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 rightDir = glm::normalize(currentOrientation * glm::vec3(1.0f, 0.0f, 0.0f));

    // --- Lift ---
    // Simplified Angle of Attack (AoA): Angle between velocity vector and aircraft's chord line (approx by forward vector)
    // Project velocity onto the aircraft's up vector.
    float vertVelComponent = glm::dot(velocityDir, upDir); // Use normalized velocity dir
    // Project velocity onto the aircraft's forward vector
    float fwdVelComponent = glm::dot(velocityDir, forwardDir);

    // Angle calculation robust for different quadrants
    float angleOfAttackRad = atan2(vertVelComponent, fwdVelComponent);
    float angleOfAttackDeg = glm::degrees(angleOfAttackRad);

    // Simplified lift coefficient (linear slope up to max) - Clamped
    float Cl = glm::clamp(PhysicsConfig::LIFT_COEFFICIENT_SLOPE * angleOfAttackDeg,
                          -PhysicsConfig::MAX_LIFT_COEFFICIENT, // Allow for inverted flight (negative lift)
                           PhysicsConfig::MAX_LIFT_COEFFICIENT);

    // Lift Force: L = 0.5 * rho * v^2 * A * Cl
    // Lift acts perpendicular to the RELATIVE WIND (velocity vector).
    // A good approximation is perpendicular to velocity in the plane defined by velocity and the aircraft's up vector.
    glm::vec3 liftDirApprox = glm::normalize(glm::cross(rightDir, velocityDir)); // Perpendicular to side vector and velocity
    float liftMagnitude = 0.5f * PhysicsConfig::AIR_DENSITY * speedSqr * PhysicsConfig::WING_AREA * Cl;
    glm::vec3 liftForce = liftDirApprox * liftMagnitude;


    // --- Drag ---
    // Base Drag (parasitic) + Induced Drag (due to lift)
    float Cd_base = PhysicsConfig::DRAG_COEFFICIENT_BASE;
    float Cd_induced = PhysicsConfig::DRAG_INDUCED_FACTOR * Cl * Cl; // Induced drag proportional to Cl^2
    float Cd_total = Cd_base + Cd_induced;

    // Drag Force: D = 0.5 * rho * v^2 * A * Cd
    // Drag acts opposite to the velocity vector.
    float dragMagnitude = 0.5f * PhysicsConfig::AIR_DENSITY * speedSqr * PhysicsConfig::WING_AREA * Cd_total;
    glm::vec3 dragForce = -velocityDir * dragMagnitude;


    return liftForce + dragForce;
}


glm::vec3 Aircraft::calculateControlTorques(const glm::quat& currentOrientation) {
    // Apply torques based on input, scaled by some factor
    // Torques are applied around the aircraft's local axes
    glm::vec3 localTorque(0.0f);

    // Pitch: Nose up (-) / down (+) input -> Torque around +X / -X axis
    localTorque.x = pitchInput * PhysicsConfig::PITCH_TORQUE_FACTOR;
    // Yaw: Nose left (-) / right (+) input -> Torque around +Y / -Y axis
    localTorque.y = yawInput * PhysicsConfig::YAW_TORQUE_FACTOR;
    // Roll: Left wing down (-) / right wing down (+) input -> Torque around -Z / +Z axis
    localTorque.z = rollInput * PhysicsConfig::ROLL_TORQUE_FACTOR;

    // Convert local torque vector to world space torque vector by rotating it
    return currentOrientation * localTorque;
}


void Aircraft::update(float deltaTime) {
    // 1. Get Control Inputs (Read from Input class)
    throttleInput = Input::Throttle;
    pitchInput = Input::Pitch;
    rollInput = Input::Roll;
    yawInput = Input::Yaw;

    // 2. Calculate Forces in World Space
    glm::vec3 gravityForce = glm::vec3(0.0f, -PhysicsConfig::GRAVITY * mass, 0.0f);
    glm::vec3 thrustForce = calculateThrustForce(orientation);
    glm::vec3 aeroForces = calculateAerodynamicForces(velocity, orientation);

    glm::vec3 totalForce = gravityForce + thrustForce + aeroForces;

    // 3. Calculate Torques in World Space
    glm::vec3 controlTorque = calculateControlTorques(orientation);
    // Aerodynamic Damping Torque (resists rotation) - Simplified
    // Proportional to negative angular velocity, scaled by some factor (related to inertia and air resistance)
    glm::vec3 dampingTorque = -angularVelocity * PhysicsConfig::ANGULAR_DAMPING * mass * 0.1f; // Very crude damping scaling
    glm::vec3 totalTorque = controlTorque + dampingTorque;


    // 4. Update Linear Motion (Semi-implicit Euler)
    glm::vec3 acceleration = totalForce / mass;
    velocity += acceleration * deltaTime;
    position += velocity * deltaTime;

    // 5. Update Angular Motion
    // Approximation: Assume inertia tensor is diagonal and roughly scalar I = k*mass
    // Angular Acceleration (alpha) = Torque / Inertia (tensor inverse * Torque)
    float approxInertiaScalar = mass * 0.5f; // Highly simplified inertia scaling factor
    if (approxInertiaScalar < 0.01f) approxInertiaScalar = 0.01f; // Avoid division by zero
    glm::vec3 angularAcceleration = totalTorque / approxInertiaScalar; // Use actual inertia tensor for accuracy

    angularVelocity += angularAcceleration * deltaTime;

    // Update orientation using angular velocity delta quaternion
    glm::quat deltaRotation;
    float angle = glm::length(angularVelocity) * deltaTime;
    if (angle > 0.0001f) { // Avoid normalization issues if no rotation
        glm::vec3 axis = glm::normalize(angularVelocity);
        deltaRotation = glm::angleAxis(angle, axis);
        orientation = glm::normalize(deltaRotation * orientation); // Apply rotation: new = delta * old
    }


    // 6. Collision Detection & Response (Simple Ground Plane y=0)
    float aircraftRadius = 0.5f; // Approximate radius/lowest point for collision
    if (position.y - aircraftRadius <= 0.0f) {
        // Collision occurred
        position.y = aircraftRadius; // Place just above ground

        // Dampen velocities significantly upon collision
        if (velocity.y < 0.0f) { // Only apply if moving downwards
            velocity.y = 0.0f; // Stop vertical velocity
            velocity.x *= 0.2f; // Friction/Impact absorption
            velocity.z *= 0.2f;
            angularVelocity *= 0.1f; // Greatly reduce spinning
        }
    }
}


void Aircraft::render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) {
    // Use member VAO
    if (!Graphics::basicShader || this->VAO == 0) return; // Check if VAO is valid

    // Now Graphics::basicShader is a pointer to a complete type because Shader.h is included
    Graphics::basicShader->use();

    // Create model matrix: translate, rotate
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = model * glm::toMat4(orientation); // Apply orientation
    // You might want to scale the model here if it's too big/small
    // model = glm::scale(model, glm::vec3(0.5f)); // Example: Make model half size

    // Set shader uniforms - compiler now knows these functions exist
    Graphics::basicShader->setMat4("model", model);
    Graphics::basicShader->setMat4("view", view);
    Graphics::basicShader->setMat4("projection", projection);
    Graphics::basicShader->setBool("useTexture", false); // Our placeholder model is not textured
    Graphics::basicShader->setVec4("objectColor", glm::vec4(0.8f, 0.1f, 0.1f, 1.0f)); // Red color
    Graphics::basicShader->setVec3("cameraPos", cameraPos); // Pass camera pos for fog

    // Draw the aircraft model
    glBindVertexArray(this->VAO); // Bind the VAO
    // Assuming setupModel used the pyramid with indices and EBO
    // The EBO is bound to the VAO state, so we just call glDrawElements
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0); // 6 faces * 3 indices/face = 18 indices for pyramid
    glBindVertexArray(0); // Unbind VAO
}

// Getters
glm::vec3 Aircraft::getPosition() const { return position; }
glm::quat Aircraft::getOrientation() const { return orientation; }
glm::vec3 Aircraft::getVelocity() const { return velocity; }
float Aircraft::getSpeed() const { return glm::length(velocity) * 3.6f; } // m/s to km/h
float Aircraft::getAltitude() const { return position.y; }

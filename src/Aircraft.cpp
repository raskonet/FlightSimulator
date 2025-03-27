#include "Aircraft.h"
#include "Graphics.h"
#include "Input.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

// Define the static Airfoil objects (linked against PhysicsConfig.cpp data)
Airfoil Aircraft::airfoil_naca0012(PhysicsConfig::NACA_0012_DATA);
Airfoil Aircraft::airfoil_naca2412(PhysicsConfig::NACA_2412_DATA);


// Constructor - Initializes RigidBody and Aircraft components
Aircraft::Aircraft(float aircraft_mass,
                   const glm::mat3& inertia_tensor,
                   Engine aircraft_engine,
                   std::vector<WingPtr> aircraft_wings)
    : engine(aircraft_engine), // Initialize engine member
      wings(std::move(aircraft_wings)) // Move ownership of wings into the member vector
{
    // Initialize RigidBody properties
    mass = aircraft_mass;
    setInertiaTensor(inertia_tensor); // Use setter to calculate inverse

    // Initial state (can be set later)
    position_world = glm::vec3(0.0f, 1000.0f, 0.0f);
    velocity_world = glm::vec3(150.0f, 0.0f, 0.0f); // Start with some forward speed (e.g., 150 m/s)
    orientation_world = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Level flight
    angular_velocity_body = glm::vec3(0.0f);
    apply_gravity = true;

    // Find control surfaces pointers based on names given during wing creation
    findControlSurfaces();

    // Setup the graphical model (same as before)
    setupModel();
}

// Helper to find wings by name
void Aircraft::findControlSurfaces() {
    for (const auto& wing : wings) {
        if (wing->name == "Left Aileron") left_aileron = wing.get();
        else if (wing->name == "Right Aileron") right_aileron = wing.get();
        else if (wing->name == "Elevator") elevator = wing.get();
        else if (wing->name == "Rudder") rudder = wing.get();
    }
    // Add warnings if any weren't found
    if (!left_aileron || !right_aileron) std::cerr << "Warning: Ailerons not found by name." << std::endl;
    if (!elevator) std::cerr << "Warning: Elevator not found by name." << std::endl;
    if (!rudder) std::cerr << "Warning: Rudder not found by name." << std::endl;
}


// Process user input and apply to engine/control surfaces
void Aircraft::processInputs(float dt) {
    // --- Throttle ---
    // Smooth throttle changes slightly? Or direct map? Let's use direct for now.
    engine.setThrottle(Input::Throttle);

    // --- Control Surfaces ---
    // Map Input::Pitch, Roll, Yaw (-1 to 1) to wing control inputs
    float roll_input = Input::Roll;
    float pitch_input = Input::Pitch;
    float yaw_input = Input::Yaw;

    // Ailerons: Roll input affects left and right ailerons differentially
    if (left_aileron) left_aileron->setControlInput(roll_input); // Left aileron up for right roll (+)
    if (right_aileron) right_aileron->setControlInput(-roll_input); // Right aileron down for right roll (+)

    // Elevator: Pitch input affects elevator directly
    // Nose down (+) pitch input means elevator goes up (-) ? Check convention.
    // Let's assume positive pitch input = elevator trailing edge down = positive control input
    if (elevator) elevator->setControlInput(pitch_input);

    // Rudder: Yaw input affects rudder directly
    // Positive yaw input (nose right) means rudder trailing edge goes right = positive control input?
    if (rudder) rudder->setControlInput(yaw_input);

}


// Aircraft's main update loop - Overrides RigidBody::update
void Aircraft::update(float dt) {
    // 1. Process Inputs -> Set Engine Throttle & Wing Controls
    processInputs(dt);

    // 2. Apply Engine Force
    engine.applyForce(this); // 'this' is the RigidBody pointer

    // 3. Apply Aerodynamic Forces from Wings
    for (const auto& wing : wings) {
        // Determine max deflection angle based on surface type (rough guess)
        float max_deflection = 20.0f;
        if (wing.get() == elevator) max_deflection = PhysicsConfig::MAX_ELEVATOR_DEFLECTION_DEG;
        else if (wing.get() == rudder) max_deflection = PhysicsConfig::MAX_RUDDER_DEFLECTION_DEG;
        else if (wing.get() == left_aileron || wing.get() == right_aileron) max_deflection = PhysicsConfig::MAX_AILERON_DEFLECTION_DEG;

        wing->applyForces(this, max_deflection);
    }

    // 4. Call the Base RigidBody Update to Integrate Physics
    RigidBody::update(dt);

    // 5. Post-Physics Checks / Clamping (Optional)
    // e.g., limit excessive spin rates, check altitude bounds etc.
     if (position_world.y < 0.5f && velocity_world.y < 0.0f) {
          // Crude ground collision handling (similar to before, but integrated better)
          position_world.y = 0.5f;
          velocity_world.y = 0.0f;
          velocity_world.x *= 0.5f;
          velocity_world.z *= 0.5f;
          angular_velocity_body *= 0.1f;
     }
}


// Rendering - Uses position_world and orientation_world from RigidBody base
void Aircraft::render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) {
    if (!Graphics::basicShader || this->VAO == 0) return;

    Graphics::basicShader->use();

    // Model matrix uses RigidBody state
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position_world);     // Use position_world
    model = model * glm::toMat4(orientation_world); // Use orientation_world
    // Optional scaling
    // model = glm::scale(model, glm::vec3(5.0f)); // Scale model UP if needed

    // Set shader uniforms
    Graphics::basicShader->setMat4("model", model);
    Graphics::basicShader->setMat4("view", view);
    Graphics::basicShader->setMat4("projection", projection);
    Graphics::basicShader->setBool("useTexture", false);
    Graphics::basicShader->setVec4("objectColor", glm::vec4(0.8f, 0.8f, 0.9f, 1.0f)); // Light grey/white color
    Graphics::basicShader->setVec3("cameraPos", cameraPos);

    // Draw the model
    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0); // Use correct index count for pyramid
    glBindVertexArray(0);
}

// Getters using RigidBody state
float Aircraft::getSpeed() const {
    return glm::length(velocity_world) * 3.6f; // m/s to km/h
}

float Aircraft::getAltitude() const {
    return position_world.y; // Directly from RigidBody state
}


// setupModel remains largely the same as before (setting up VAO/VBO/EBO for pyramid)
void Aircraft::setupModel() {
    float simple_pyramid_vertices[] = {
        -0.5f, -0.25f, -0.5f,  0.0f, 0.0f, // 0
         0.5f, -0.25f, -0.5f,  1.0f, 0.0f, // 1
         0.5f, -0.25f,  0.5f,  1.0f, 1.0f, // 2
        -0.5f, -0.25f,  0.5f,  0.0f, 1.0f, // 3
         0.0f,  0.75f,  0.0f,  0.5f, 0.5f  // 4
     };
     unsigned int simple_pyramid_indices[] = {
        0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4, // Sides
        3, 2, 0, 2, 1, 0                      // Base
     };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(simple_pyramid_vertices), simple_pyramid_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(simple_pyramid_indices), simple_pyramid_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // Note: EBO remains bound to VAO state implicitly
}

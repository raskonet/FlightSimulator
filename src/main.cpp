#include "Graphics.h"
#include "Aircraft.h"
#include "Terrain.h"    // <-- Include Terrain
#include "MiniMap.h"
#include "Camera.h"
#include "Input.h"
#include "Shader.h"
#include "PhysicsConfig.h" // For aircraft setup if needed here
#include <iostream>
#include <memory>
#include <vector>
#include <stdexcept> // Needed for try/catch

void renderUI(const Aircraft& aircraft); // Forward declare

int main() {
    try { // Add a try-catch block for easier error handling during init
        // --- Initialization ---
        if (!Graphics::init(1600, 900, "Flight Simulator")) { // Use a good resolution
            std::cerr << "Failed to initialize Graphics!" << std::endl;
            return -1;
        }

        // --- Create Aircraft (as before) ---
        Engine engine(PhysicsConfig::DEFAULT_THRUST);
        std::vector<WingPtr> wings;
        auto addWing = [&](const std::string& name, const glm::vec3& pos, float span, float chord, const Airfoil* foil, const glm::vec3& normal = PhysicsConfig::BODY_UP, float flapRatio = 0.0f) {
            if (!foil) throw std::runtime_error("Null airfoil for " + name);
            wings.push_back(std::make_unique<Wing>(name, pos, span, chord, foil, normal, flapRatio));
        };
        addWing("Left Wing",       PhysicsConfig::LEFT_WING_POS,      6.96f, 2.50f, &Aircraft::airfoil_naca2412);
        addWing("Right Wing",      PhysicsConfig::RIGHT_WING_POS,     6.96f, 2.50f, &Aircraft::airfoil_naca2412);
        addWing("Left Aileron",    PhysicsConfig::LEFT_AILERON_POS,   3.80f, 1.26f, &Aircraft::airfoil_naca0012, PhysicsConfig::BODY_UP, 1.0f);
        addWing("Right Aileron",   PhysicsConfig::RIGHT_AILERON_POS,  3.80f, 1.26f, &Aircraft::airfoil_naca0012, PhysicsConfig::BODY_UP, 1.0f);
        addWing("Elevator",        PhysicsConfig::ELEVATOR_POS,       6.54f, 2.70f, &Aircraft::airfoil_naca0012, PhysicsConfig::BODY_UP, 1.0f);
        addWing("Rudder",          PhysicsConfig::RUDDER_POS,         5.31f, 3.10f, &Aircraft::airfoil_naca0012, PhysicsConfig::BODY_RIGHT, 1.0f);

        Aircraft aircraft(
            PhysicsConfig::DEFAULT_MASS,
            PhysicsConfig::DEFAULT_INERTIA_TENSOR,
            engine,
            std::move(wings)
        );
        aircraft.position_world = glm::vec3(0.0f, 1000.0f, 0.0f);
        aircraft.velocity_world = glm::vec3(180.0f, 0.0f, 0.0f);
        aircraft.orientation_world = glm::quatLookAt(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));


        // --- Create Terrain ---
        Terrain terrain; // Instantiate the new terrain system


        // --- Other Game Objects ---
        MiniMap miniMap;
        Camera camera(aircraft.position_world + glm::vec3(-20.0f, 10.0f, 0.0f)); // Adjusted start camera pos


        // --- Timing ---
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;

        // --- Main Loop ---
        while (!Graphics::shouldClose()) {
            // --- Timing ---
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            if (deltaTime > 0.1f) deltaTime = 0.1f;
            if (deltaTime <= 0.0f) deltaTime = 0.0001f;

            // --- Input ---
            Input::ProcessInput(Graphics::getWindow());

            // --- Update ---
            aircraft.update(deltaTime);

            // --- Camera Update ---
            camera.Follow(aircraft.position_world, aircraft.orientation_world, 25.0f, 10.0f); // Adjusted follow

            // --- Rendering ---
            Graphics::clear();

            int screenWidth = Graphics::getWidth();
            int screenHeight = Graphics::getHeight();
            float aspectRatio = (screenHeight > 0) ? (float)screenWidth / (float)screenHeight : 1.0f;
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspectRatio, 0.5f, 80000.0f); // Very large far plane for terrain

            glm::mat4 view = camera.GetViewMatrix();

            // --- Render Terrain ---
            glm::vec3 sunDirection = glm::normalize(glm::vec3(-0.4f, -0.8f, -0.2f)); // Example sun direction
            terrain.draw(camera, projection, sunDirection);

            // --- Render Aircraft ---
            if (Graphics::basicShader) {
                Graphics::basicShader->use();
                Graphics::basicShader->setVec3("cameraPos", camera.Position);
                Graphics::basicShader->setVec3("fogColor", glm::vec3(0.5f, 0.6f, 0.7f));
                Graphics::basicShader->setFloat("fogDensity", 0.00005f); // Very low density
                // Aircraft::render sets its own view/projection uniforms
                aircraft.render(view, projection, camera.Position);
                Graphics::basicShader->use(false);
            }

            // --- Render 2D Overlays ---
            // Use terrain size or a large fixed value for minimap scale
            miniMap.render(aircraft.position_world, aircraft.orientation_world, terrain.getTerrainSize());
            renderUI(aircraft);

            // --- Swap Buffers & Poll Events ---
            Graphics::swapBuffers();
        }

        // --- Cleanup ---
        Graphics::cleanup(); // Handles basicShader etc.

    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        Graphics::cleanup(); // Attempt cleanup even on error
        return -1;
    } catch (...) {
         std::cerr << "FATAL UNKNOWN ERROR occurred." << std::endl;
         Graphics::cleanup();
         return -1;
    }

    std::cout << "Flight Simulator terminated cleanly." << std::endl;
    return 0;
}

// ... renderUI function ...
void renderUI(const Aircraft& aircraft) { /* ... placeholder ... */ }

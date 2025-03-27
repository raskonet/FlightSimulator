// #define STB_IMAGE_IMPLEMENTATION // REMOVED - Defined via CMake now
#include <stb_image.h> // Keep this include for the header definitions

#include "Graphics.h"   // Must be included before others that use GL types indirectly
#include "Aircraft.h"
#include "Map.h"
#include "MiniMap.h"
#include "Camera.h"
#include "Input.h"
#include "Shader.h"     // <-- ADDED THIS INCLUDE for full Shader definition
#include <iostream>
#include <memory>       // For unique_ptr

// Function prototype for UI rendering (placeholder)
void renderUI(const Aircraft& aircraft);

int main() {
    // --- Initialization ---
    if (!Graphics::init(1280, 720, "Flight Simulator")) {
        std::cerr << "Failed to initialize Graphics!" << std::endl;
        return -1;
    }

    // Create game objects
    Aircraft aircraft(glm::vec3(0.0f, 100.0f, 0.0f)); // Start 100m up
    Map map;                                          // Needs definition before getMapSize() is called
    MiniMap miniMap;
    Camera camera(glm::vec3(0.0f, 105.0f, 10.0f));    // Initial camera position slightly behind and above

    // Timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // --- Main Loop ---
    while (!Graphics::shouldClose()) {
        // --- Timing ---
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Prevent spiral of death if deltaTime is too large (e.g., breakpoint hit, heavy load)
        if (deltaTime > 0.1f) deltaTime = 0.1f;
        // Avoid zero or negative delta time if clock goes backward? (unlikely but safe)
        if (deltaTime <= 0.0f) deltaTime = 0.0001f;


        // --- Input ---
        Input::ProcessInput(Graphics::getWindow()); // Read key states into Input class static members

        // --- Update ---
        aircraft.update(deltaTime); // Update aircraft physics and state

        // Update camera to follow aircraft
        camera.Follow(aircraft.getPosition(), aircraft.getOrientation(), 15.0f, 5.0f); // Follow 15m behind, 5m above


        // --- Rendering ---
        Graphics::clear(); // Clear screen (color and depth)

        // Setup projection matrix (aspect ratio depends on potentially resized window)
        int screenWidth = Graphics::getWidth();
        int screenHeight = Graphics::getHeight();
        float aspectRatio = (screenHeight > 0) ? (float)screenWidth / (float)screenHeight : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                               aspectRatio,
                                               0.1f,
                                               map.getMapSize() * 1.5f); // <-- Use getter for mapSize

        // Setup view matrix
        glm::mat4 view = camera.GetViewMatrix();

        // Set uniforms that are common to the basic shader (like fog) once
        if (Graphics::basicShader) {
            Graphics::basicShader->use(); // Activate shader to set uniforms
            Graphics::basicShader->setVec3("cameraPos", camera.Position);
            // Set fog parameters (can be done once or per frame if they change)
            Graphics::basicShader->setVec3("fogColor", glm::vec3(0.5f, 0.6f, 0.7f));
            Graphics::basicShader->setFloat("fogDensity", 0.005f); // Adjust density
        } else {
             std::cerr << "ERROR: Basic shader not initialized! Cannot render 3D scene." << std::endl;
             // Handle error appropriately, maybe break loop or skip rendering
             break; // Exit loop if shader is crucial and missing
        }

        // Render 3D scene - pass necessary matrices AND camera position
        map.render(view, projection, camera.Position); // <-- Pass camera position
        aircraft.render(view, projection, camera.Position); // <-- Pass camera position


        // Render 2D overlays (MiniMap, UI) - these typically use different shaders/projections
        miniMap.render(aircraft.getPosition(), aircraft.getOrientation(), map.getMapSize()); // <-- Use getter
        renderUI(aircraft); // Render placeholder UI text (needs implementation)


        // --- Swap Buffers & Poll Events ---
        Graphics::swapBuffers();
    }

    // --- Cleanup ---
    // Objects with OpenGL resources (Aircraft, Map, MiniMap) clean up via destructors (RAII)
    // Graphics::cleanup handles GLFW, GLEW, Shaders managed by Graphics
    Graphics::cleanup();

    std::cout << "Flight Simulator terminated cleanly." << std::endl;
    return 0;
}


// Placeholder UI Rendering function
// Needs a proper text rendering library (e.g., FreeType + GLM, Dear ImGui)
void renderUI(const Aircraft& aircraft) {
    // Example: Get data from aircraft
    float speed = aircraft.getSpeed();
    float altitude = aircraft.getAltitude();
    float throttle = Input::Throttle; // Directly access static input state

    // --- How to render text (Conceptual) ---
    // 1. Set up Orthographic projection for 2D rendering (usually 0,0 top-left)
    // 2. Use a text rendering library (like ImGui::Text or your own FreeType renderer)
    //    - Generate formatted strings (e.g., using snprintf or std::format)
    //    - Call the library's functions to draw the text at specific screen coordinates

    // Pseudo-code using an imaginary TextRenderer class:
    /*
    TextRenderer::BeginFrame(); // Setup state if needed

    char buffer[100];

    snprintf(buffer, sizeof(buffer), "Speed: %.0f km/h", speed);
    TextRenderer::DrawText(buffer, 10.0f, 10.0f, glm::vec3(1.0f, 1.0f, 1.0f)); // x, y, color

    snprintf(buffer, sizeof(buffer), "Altitude: %.0f m", altitude);
    TextRenderer::DrawText(buffer, 10.0f, 30.0f, glm::vec3(1.0f, 1.0f, 1.0f));

    snprintf(buffer, sizeof(buffer), "Throttle: %.0f %%", throttle * 100.0f);
    TextRenderer::DrawText(buffer, 10.0f, 50.0f, glm::vec3(1.0f, 1.0f, 1.0f));

    TextRenderer::EndFrame(); // Draw batched text / cleanup state
    */

    // The std::cout logging can still be useful for debugging but won't show in the window.
}

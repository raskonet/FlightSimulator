#include "Input.h"
#include <iostream> // For debugging

// Initialize static members
float Input::Throttle = 0.0f;
float Input::Pitch = 0.0f;
float Input::Roll = 0.0f;
float Input::Yaw = 0.0f;
std::map<int, bool> Input::keys;

void Input::Initialize(GLFWwindow* window) {
    glfwSetKeyCallback(window, keyCallback);
    // Could add mouse callbacks here too if needed
}

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Close window on Escape press
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }

    // Update key state map
    if (action == GLFW_PRESS) {
        keys[key] = true;
    } else if (action == GLFW_RELEASE) {
        keys[key] = false;
    }
}

void Input::ProcessInput(GLFWwindow *window) {
    // Reset axes that depend on continuous press
    Pitch = 0.0f;
    Roll = 0.0f;
    Yaw = 0.0f;

    // Throttle (Increase/Decrease)
    if (keys[GLFW_KEY_W]) Throttle += 0.5f; // Use a delta, clamp later
    if (keys[GLFW_KEY_S]) Throttle -= 0.5f;

    // Clamp Throttle
    if (Throttle < 0.0f) Throttle = 0.0f;
    if (Throttle > 1.0f) Throttle = 1.0f;

    // Pitch (Up/Down)
    if (keys[GLFW_KEY_DOWN]) Pitch = 1.0f;  // Nose down
    if (keys[GLFW_KEY_UP]) Pitch = -1.0f; // Nose up

    // Roll (Left/Right)
    if (keys[GLFW_KEY_LEFT]) Roll = -1.0f; // Roll left
    if (keys[GLFW_KEY_RIGHT]) Roll = 1.0f; // Roll right

    // Yaw (Q/E)
    if (keys[GLFW_KEY_Q]) Yaw = -1.0f; // Yaw left
    if (keys[GLFW_KEY_E]) Yaw = 1.0f;  // Yaw right

    // Debugging output (optional)
    // std::cout << "Throttle: " << Throttle << " Pitch: " << Pitch << " Roll: " << Roll << " Yaw: " << Yaw << std::endl;
}

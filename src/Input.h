#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include <map>

class Input {
public:
    // Control states
    static float Throttle; // 0.0 to 1.0
    static float Pitch;    // -1.0 (down) to 1.0 (up)
    static float Roll;     // -1.0 (left) to 1.0 (right)
    static float Yaw;      // -1.0 (left) to 1.0 (right)

    static void Initialize(GLFWwindow* window);
    static void ProcessInput(GLFWwindow *window); // Process continuous key presses

private:
    // Callback function for key presses/releases
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    // Track which keys are currently held down
    static std::map<int, bool> keys;
};

#endif // INPUT_H

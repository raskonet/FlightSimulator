#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <GL/glew.h> // Must be included before GLFW
#include <GLFW/glfw3.h>
#include <string>
#include <memory> // For unique_ptr

// Forward declarations
class Shader;
class Camera;

class Graphics {
public:
    static bool init(int width = 1280, int height = 720, const std::string& title = "Flight Simulator");
    static void cleanup();
    static void clear();
    static void swapBuffers();
    static bool shouldClose();
    static GLFWwindow* getWindow(); // Make window accessible if needed
    static int getWidth();
    static int getHeight();

    // Manage Shaders (can be expanded)
    static std::unique_ptr<Shader> basicShader;
    static std::unique_ptr<Shader> minimapShader;

private:
    static GLFWwindow* window;
    static int screenWidth;
    static int screenHeight;

    // Callback for window resize
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

#endif // GRAPHICS_H

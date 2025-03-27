#include "Graphics.h"
#include "Shader.h" // Include Shader header
#include "Input.h"  // Include Input for initialization
#include <iostream>

// Initialize static members
GLFWwindow* Graphics::window = nullptr;
int Graphics::screenWidth = 0;
int Graphics::screenHeight = 0;
std::unique_ptr<Shader> Graphics::basicShader = nullptr;
std::unique_ptr<Shader> Graphics::minimapShader = nullptr;


bool Graphics::init(int width, int height, const std::string& title) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // Set resize callback

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    // Set initial viewport size
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D
    glEnable(GL_BLEND); // Enable blending for potential transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load Shaders
    basicShader = std::make_unique<Shader>("assets/shaders/basic_shader.vert", "assets/shaders/basic_shader.frag");
    minimapShader = std::make_unique<Shader>("assets/shaders/minimap_shader.vert", "assets/shaders/minimap_shader.frag");

    if (!basicShader->ID || !minimapShader->ID) {
         std::cerr << "Failed to load shaders." << std::endl;
         cleanup(); // Cleanup already initialized resources
         return false;
    }

    // Initialize Input system
    Input::Initialize(window);

    std::cout << "Graphics Initialized Successfully. OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    return true;
}

void Graphics::cleanup() {
    // Shaders cleaned up by unique_ptr automatically
    basicShader.reset();
    minimapShader.reset();

    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
    std::cout << "Graphics Cleaned Up." << std::endl;
}

void Graphics::clear() {
    // Set clear color (sky blue)
    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Graphics::swapBuffers() {
    glfwSwapBuffers(window);
    glfwPollEvents(); // Poll for input events
}

bool Graphics::shouldClose() {
    return glfwWindowShouldClose(window);
}

GLFWwindow* Graphics::getWindow() {
    return window;
}

int Graphics::getWidth() {
    return screenWidth;
}

int Graphics::getHeight() {
    return screenHeight;
}

// Callback function for when the window is resized
void Graphics::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    screenWidth = width;
    screenHeight = height;
     std::cout << "Window resized to " << width << "x" << height << std::endl;
}

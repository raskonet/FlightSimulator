cmake_minimum_required(VERSION 3.10)
project(FlightSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Dependencies ---
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED) # Added GLEW
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# --- Include Directories ---
include_directories(src vendor) # Added vendor for stb_image

# --- Source Files ---
set(SOURCES
    src/main.cpp
    src/Aircraft.cpp
    src/Map.cpp
    src/MiniMap.cpp
    src/Graphics.cpp
    src/Camera.cpp      # Added
    src/Shader.cpp      # Added
    src/Input.cpp       # Added
    src/Texture.cpp     # Added
)

# --- Executable ---
add_executable(FlightSimulator ${SOURCES})

# --- Linking ---
target_link_libraries(FlightSimulator
    PRIVATE
    OpenGL::GL
    GLEW::GLEW      # Link GLEW
    glfw
    glm::glm        # Use modern glm target
)

# --- Optional: Copy Assets ---
# Add custom command to copy assets directory to build directory if needed
# file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# --- STB Image Implementation ---
# Tell CMake that stb_image is header-only but needs one source file
# to compile the implementation. We do this in main.cpp.
# target_compile_definitions(FlightSimulator PRIVATE STB_IMAGE_IMPLEMENTATION)

# --- Platform Specific ---
if(APPLE)
    target_link_libraries(FlightSimulator "-framework CoreFoundation")
endif()

message(STATUS "OpenGL Version: ${OPENGL_VERSION_STRING}")
message(STATUS "GLEW Found: ${GLEW_FOUND}")
message(STATUS "GLFW Found: ${GLFW3_FOUND}")
message(STATUS "GLM Found: ${GLM_FOUND}")

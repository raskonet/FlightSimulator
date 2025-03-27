cmake_minimum_required(VERSION 3.10)
project(FlightSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Dependencies ---
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# --- Include Directories ---
include_directories(src vendor)

# --- Source Files ---
set(SOURCES
    src/main.cpp
    src/Graphics.cpp
    src/Camera.cpp
    src/Shader.cpp
    src/Input.cpp
    src/Texture.cpp         # ADD Texture.cpp
    # src/Map.cpp           # REMOVE Map.cpp
    src/Terrain.cpp         # ADD Terrain.cpp
    src/MiniMap.cpp
    # --- Physics Files ---
    src/PhysicsConfig.cpp
    src/RigidBody.cpp
    src/Airfoil.cpp
    src/Wing.cpp
    # --- Aircraft (Modified) ---
    src/Aircraft.cpp
) # Note: OpenGLUtils.h and TerrainBlock.h are header-only

# --- Executable ---
add_executable(FlightSimulator ${SOURCES})

# --- Linking ---
target_link_libraries(FlightSimulator
    PRIVATE
    OpenGL::GL
    GLEW::GLEW
    glfw
    glm::glm
)

# --- STB Image Implementation (Defined manually in Texture.cpp now) ---
# REMOVED: target_compile_definitions(FlightSimulator PRIVATE STB_IMAGE_IMPLEMENTATION)

# --- Optional: Copy Assets ---
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
message(STATUS "Copying 'assets' directory to ${CMAKE_BINARY_DIR}")

# --- Platform Specific ---
if(APPLE)
    target_link_libraries(FlightSimulator "-framework CoreFoundation")
endif()

message(STATUS "OpenGL Version: ${OPENGL_VERSION_STRING}")
message(STATUS "GLEW Found: ${GLEW_FOUND}")
message(STATUS "GLFW Found: ${GLFW3_FOUND}")
message(STATUS "GLM Found: ${GLM_FOUND}")

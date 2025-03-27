#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include "RigidBody.h"     // Inherit from RigidBody
#include "Engine.h"
#include "Airfoil.h"       // Include Airfoil type definition
#include "PhysicsConfig.h" // Include PhysicsConfig for defaults etc.
#include "Wing.h"          // <-- ***** ADDED: Need full Wing definition for unique_ptr *****
#include <vector>
#include <memory>         // <-- ***** ADDED: For unique_ptr *****
#include <string>         // For wing names

// Forward declarations
class Shader;             // Still needed for render function signature

// Required for GLuint type for rendering members
#include <GL/glew.h>

// --- Type alias for unique pointer to Wing ---
// Define *before* Aircraft class
using WingPtr = std::unique_ptr<Wing>; // <-- ***** MOVED & ENSURED Wing.h/memory are included first *****

// Rename/refactor Aircraft to Airplane conceptually, inherits RigidBody
class Aircraft : public RigidBody {
public: // <-- ***** MOVED STATIC AIRFOILS TO PUBLIC *****
    // Static airfoil objects (defined in Aircraft.cpp, linked against PhysicsConfig.cpp data)
    // Must be accessible by main.cpp to pass pointers when creating wings.
    static Airfoil airfoil_naca0012;
    static Airfoil airfoil_naca2412;

    // --- Components ---
    Engine engine;
    std::vector<WingPtr> wings; // Use the WingPtr alias

    // Pointers to specific control surfaces for easier access (optional)
    Wing* left_aileron = nullptr;
    Wing* right_aileron = nullptr;
    Wing* elevator = nullptr;
    Wing* rudder = nullptr;

    // --- Constructor ---
    Aircraft(float aircraft_mass,
             const glm::mat3& inertia_tensor,
             Engine aircraft_engine,
             std::vector<WingPtr> aircraft_wings); // Takes vector of wing unique_ptrs

    // Destructor override if needed (unique_ptr handles wing cleanup automatically)
    virtual ~Aircraft() override = default;

    // --- Simulation Update Override ---
    virtual void update(float dt) override;

    // --- Rendering (Keep existing structure) ---
    void render(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);
    float getSpeed() const; // km/h
    float getAltitude() const; // meters

private: // <-- ***** KEEP RENDERING/INTERNAL STUFF PRIVATE *****
    // Rendering resources
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    void setupModel(); // Sets up the VAO/VBO/EBO

    // --- Input Processing Helper ---
    void processInputs(float dt);

    // Helper to find control surfaces by name during construction
    void findControlSurfaces();
};

#endif // AIRCRAFT_H

#ifndef WING_H
#define WING_H

#include "RigidBody.h"
#include "Airfoil.h"
#include "PhysicsConfig.h" // For constants, sq(), PI, get_air_density()
#include <glm/glm.hpp>
#include <string>
#include <stdexcept>     // <-- ***** ADDED for std::runtime_error *****
#include <vector>        // Included by Airfoil.h usually, but safe to add
#include <cmath>         // For std::abs
#include <algorithm>     // For std::clamp

class Wing {
public:
    // --- Configuration ---
    std::string name; // Optional name for debugging
    glm::vec3 center_of_pressure_body; // Position relative to CG in body coords
    float span;         // Wingspan (m)
    float chord;        // Average chord length (m)
    const Airfoil* airfoil; // Pointer to airfoil data (must not be null)
    glm::vec3 base_normal_body; // Normal vector when undeflected (usually BODY_UP or BODY_RIGHT for rudder)

    float flap_ratio;   // Ratio of wing area affected by control surface (0 to 1)
    float efficiency_factor; // Oswald efficiency factor (e.g., 0.8)

    // --- Calculated Properties ---
    float area;         // Wing area (m^2)
    float aspect_ratio; // Aspect ratio (span^2 / area)

    // --- Control Input ---
    float control_input; // Deflection amount (-1.0 to 1.0)


    Wing(const std::string& wingName,
         const glm::vec3& position_body,
         float wing_span,
         float wing_chord,
         const Airfoil* foil, // Must be valid pointer
         const glm::vec3& normal_body = PhysicsConfig::BODY_UP,
         float wing_flap_ratio = 0.0f,
         float oswald_factor = PhysicsConfig::EFFICIENCY_FACTOR)
        : name(wingName),
          center_of_pressure_body(position_body),
          span(wing_span),
          chord(wing_chord),
          airfoil(foil), // Store the provided pointer
          base_normal_body(glm::normalize(normal_body)),
          flap_ratio(std::clamp(wing_flap_ratio, 0.0f, 1.0f)),
          efficiency_factor(oswald_factor),
          area(span * chord),
          aspect_ratio(area > 1e-6f ? PhysicsConfig::sq(span) / area : 0.0f),
          control_input(0.0f)
    {
        // Check if the provided airfoil pointer is valid
        if (!airfoil) {
             // Throw runtime error if airfoil is null
             throw std::runtime_error("Wing must be initialized with a valid Airfoil pointer.");
        }
    }

    // Set control surface deflection (-1.0 to 1.0)
    void setControlInput(float input) {
        control_input = std::clamp(input, -1.0f, 1.0f);
    }

    // Calculate and apply aerodynamic forces to the rigid body
    void applyForces(RigidBody* rigid_body, float max_deflection_angle_deg = 20.0f) const;

private:
    // Helper to calculate the effective normal vector based on control input
    glm::vec3 calculateEffectiveNormal(float max_deflection_angle_deg) const;
};

#endif // WING_H

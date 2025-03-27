#ifndef PHYSICS_CONFIG_H
#define PHYSICS_CONFIG_H

#include <glm/glm.hpp>
#include <vector>
#include <cmath> // For std::exp

namespace PhysicsConfig {
    const float GRAVITY = 9.81f;
    const float PI = 3.14159265359f;

    // --- Math Helpers ---
    inline float sq(float val) { return val * val; }

    // --- ISA (International Standard Atmosphere) Model ---
    // Simplified version - returns air density (kg/m^3) based on altitude (meters)
    // Sea level density: 1.225 kg/m^3
    // Temperature lapse rate: -0.0065 K/m up to 11km
    // Pressure = P0 * (T/T0)^(-g/(R*L))
    // Density = P / (R * T)
    // Very simplified exponential decay approximation for density:
    inline float get_air_density(float altitude_m) {
        const float seaLevelDensity = 1.225f;
        const float scaleHeight = 8500.0f; // Approximate scale height for density
        return seaLevelDensity * std::exp(-altitude_m / scaleHeight);
    }

    // --- Airfoil Data (Example NACA 0012 & 2412) ---
    // Data points: { alpha_degrees, Cl, Cd }
    // Source: airfoiltools.com (or similar standard data)
    const extern std::vector<glm::vec3> NACA_0012_DATA;
    const extern std::vector<glm::vec3> NACA_2412_DATA;

    // --- Default Aircraft Parameters (Based on Blog Example) ---
    const float DEFAULT_MASS = 10000.0f; // kg
    const float DEFAULT_THRUST = 50000.0f; // Newtons

    // Example Inertia Tensor (Body Space: X=Roll, Y=Pitch, Z=Yaw axes) - Check Axis definitions!
    // Blog likely uses different convention (X=Forward?). Assuming common flight sim:
    // X: Longitudinal (Roll), Y: Vertical (Pitch), Z: Lateral (Yaw)
    // Or X: Forward, Y: Right, Z: Down
    // Let's assume blog used X=Roll, Y=Pitch, Z=Yaw for now based on matrix values. Needs verification.
    const glm::mat3 DEFAULT_INERTIA_TENSOR = {
        // Ix,  Ixy, Ixz
        {48531.0f, -1320.0f,    0.0f},
        // Iyx, Iy,  Iyz
        {-1320.0f, 256608.0f,   0.0f},
        // Izx, Izy, Iz
        {   0.0f,    0.0f, 211333.0f}
    };

    // Wing/Engine configuration constants from blog
    const float WING_ROOT_OFFSET_X = -1.0f; // Longitudinal position relative to CG
    const float AILERON_OFFSET_X = -1.0f; // Using same X as wing root for simplicity here
    const float TAIL_OFFSET_X = -6.6f;   // Tail position relative to CG

    // Wing geometric centers (body coords relative to CG - APPROXIMATE based on blog text)
    // Assuming common aircraft coords: +X forward, +Y right, +Z down (or +Y up, +Z right?)
    // Let's assume +X forward, +Y right, +Z down for applying forces
    const glm::vec3 LEFT_WING_POS = {WING_ROOT_OFFSET_X, -2.7f, 0.0f};
    const glm::vec3 RIGHT_WING_POS = {WING_ROOT_OFFSET_X, 2.7f, 0.0f};
    const glm::vec3 LEFT_AILERON_POS = {AILERON_OFFSET_X, -4.7f, 0.0f}; // Approx outer position
    const glm::vec3 RIGHT_AILERON_POS = {AILERON_OFFSET_X, 4.7f, 0.0f}; // Approx outer position
    const glm::vec3 ELEVATOR_POS = {TAIL_OFFSET_X, 0.0f, -0.1f}; // Slightly below CG?
    const glm::vec3 RUDDER_POS = {TAIL_OFFSET_X, 0.0f, 0.5f}; // Approx vertical position on tail

    // Control surface deflection limits/factors
    const float MAX_AILERON_DEFLECTION_DEG = 20.0f;
    const float MAX_ELEVATOR_DEFLECTION_DEG = 25.0f;
    const float MAX_RUDDER_DEFLECTION_DEG = 30.0f;

    // Aerodynamic constants
    const float EFFICIENCY_FACTOR = 0.8f; // Oswald efficiency factor (e) for induced drag

    // Body Space Direction Vectors (assuming +X forward, +Y right, +Z down)
    const glm::vec3 BODY_FORWARD = {1.0f, 0.0f, 0.0f};
    const glm::vec3 BODY_RIGHT = {0.0f, 1.0f, 0.0f};
    const glm::vec3 BODY_UP = {0.0f, 0.0f, -1.0f}; // Corresponds to -Z body axis
    const glm::vec3 BODY_DOWN = {0.0f, 0.0f, 1.0f};
    const glm::vec3 BODY_LEFT = {0.0f, -1.0f, 0.0f};

} // namespace PhysicsConfig

#endif // PHYSICS_CONFIG_H

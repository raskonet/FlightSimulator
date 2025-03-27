#include "PhysicsConfig.h"

namespace PhysicsConfig {

    // Data points: { alpha_degrees, Cl, Cd } - MUST provide actual data
    // Example placeholder structure - GET REAL NACA DATA
    const std::vector<glm::vec3> NACA_0012_DATA = {
        {-180.0f, 0.0f, 0.06f}, {-90.0f, 0.0f, 1.2f}, {-20.0f, -1.2f, 0.12f},
        {-15.0f, -1.1f, 0.08f}, {-10.0f, -0.8f, 0.04f}, {-5.0f, -0.4f, 0.015f},
        {0.0f,   0.0f, 0.006f}, {5.0f,   0.4f, 0.015f}, {10.0f,  0.8f, 0.04f},
        {15.0f,  1.1f, 0.08f},  {20.0f,  1.2f, 0.12f}, {90.0f, 0.0f, 1.2f}, {180.0f, 0.0f, 0.06f}
        // Add many more points for accuracy, especially around stall angles
    };

    const std::vector<glm::vec3> NACA_2412_DATA = {
        {-180.0f, 0.0f, 0.06f}, {-90.0f, 0.0f, 1.2f}, {-15.0f, -0.8f, 0.08f},
        {-10.0f, -0.5f, 0.04f}, {-5.0f,  0.0f, 0.015f}, {0.0f,   0.25f, 0.006f}, // Cambered airfoil has lift at 0 AoA
        {5.0f,   0.7f, 0.015f}, {10.0f,  1.1f, 0.04f},  {15.0f,  1.4f, 0.08f},
        {20.0f,  1.5f, 0.15f}, {90.0f, 0.0f, 1.2f}, {180.0f, 0.0f, 0.06f}
        // Add many more points for accuracy
    };

} // namespace PhysicsConfig

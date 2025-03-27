#ifndef AIRFOIL_H
#define AIRFOIL_H

#include <vector>
#include <glm/glm.hpp>
#include <tuple> // For std::tuple
#include <algorithm> // For std::lower_bound, std::clamp
#include "PhysicsConfig.h" // For sq function maybe, or move helpers

class Airfoil {
public:
    // Data points are expected as { alpha_degrees, Cl, Cd } sorted by alpha
    Airfoil(const std::vector<glm::vec3>& curveData);

    // Get Cl and Cd for a given angle of attack (alpha) in degrees.
    // Uses linear interpolation between known data points.
    std::tuple<float, float> sample(float alpha_deg) const;

    // Find max Cl (useful for flaps) - simplistic implementation
    float getMaxCl() const;

private:
    const std::vector<glm::vec3>& data; // Reference to the constant data
    float min_alpha_deg;
    float max_alpha_deg;
};

#endif // AIRFOIL_H

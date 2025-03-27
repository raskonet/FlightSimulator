#include "Airfoil.h"
#include <stdexcept> // For exceptions
#include <limits>    // For numeric_limits

Airfoil::Airfoil(const std::vector<glm::vec3>& curveData) : data(curveData) {
    if (data.empty()) {
        throw std::runtime_error("Airfoil data cannot be empty.");
    }
    min_alpha_deg = data.front().x;
    max_alpha_deg = data.back().x;
    // Optional: Add check for sorted data here
}

std::tuple<float, float> Airfoil::sample(float alpha_deg) const {
    // Clamp alpha to the range of the data
    alpha_deg = std::clamp(alpha_deg, min_alpha_deg, max_alpha_deg);

    // Find the first data point whose alpha is not less than the input alpha
    auto it = std::lower_bound(data.begin(), data.end(), alpha_deg,
                               [](const glm::vec3& point, float alpha) {
                                   return point.x < alpha;
                               });

    // Handle edge cases: alpha is exactly the first or last point, or outside range (already clamped)
    if (it == data.begin()) {
        return {it->y, it->z}; // Return first point's Cl, Cd
    }
    if (it == data.end()) {
        return {(data.end() - 1)->y, (data.end() - 1)->z}; // Return last point's Cl, Cd
    }

    // Linear interpolation between the point before 'it' and 'it'
    const glm::vec3& p1 = *(it - 1); // Point before or equal to alpha
    const glm::vec3& p2 = *it;       // Point after alpha

    // Avoid division by zero if data points have same alpha
    if (std::abs(p2.x - p1.x) < 1e-6f) {
        return {p1.y, p1.z}; // or p2, doesn't matter much
    }

    // Calculate interpolation factor (t)
    float t = (alpha_deg - p1.x) / (p2.x - p1.x);

    // Interpolate Cl and Cd
    float Cl = p1.y + t * (p2.y - p1.y);
    float Cd = p1.z + t * (p2.z - p1.z);

    return {Cl, Cd};
}

float Airfoil::getMaxCl() const {
     // Simplistic: just find max value in data. A more robust way might involve curve fitting.
     float maxCl = -std::numeric_limits<float>::infinity();
     for(const auto& point : data) {
         if (point.y > maxCl) {
             maxCl = point.y;
         }
     }
     return (maxCl > -std::numeric_limits<float>::infinity()) ? maxCl : 1.5f; // Return default if no data
}

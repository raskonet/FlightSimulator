#include "Wing.h"
#include <cmath> // For std::asin
#include <glm/gtx/vector_angle.hpp> // For angle (maybe not needed if using asin)
#include <iostream> // For debug

glm::vec3 Wing::calculateEffectiveNormal(float max_deflection_angle_deg) const {
     if (flap_ratio <= 0.0f || std::abs(control_input) < 1e-6f) {
         return base_normal_body; // No deflection
     }

     // Calculate deflection angle in radians
     float deflection_rad = glm::radians(control_input * max_deflection_angle_deg);

     // Determine the axis of rotation for the flap
     // Assume flaps rotate around an axis parallel to the wing's leading/trailing edge
     // This axis is perpendicular to the normal and the span direction.
     // Guessing span direction is likely BODY_RIGHT or BODY_LEFT relative to normal.
     // If normal is UP, rotation axis is RIGHT. If normal is RIGHT (rudder), rotation axis is UP.
     glm::vec3 rotation_axis;
     if (glm::abs(glm::dot(base_normal_body, PhysicsConfig::BODY_UP)) > 0.9f) {
         // Wing/Elevator (Normal is roughly Up/Down) - Rotate around Body Y (Right) axis
         rotation_axis = PhysicsConfig::BODY_RIGHT;
     } else if (glm::abs(glm::dot(base_normal_body, PhysicsConfig::BODY_RIGHT)) > 0.9f) {
          // Rudder (Normal is roughly Right/Left) - Rotate around Body Z (Up/Down)? Or X (Forward)? Let's assume Body UP (Z axis in OpenGL coords?)
          // This needs careful axis definition. If BODY_UP is (0,0,-1), need rotation axis perpendicular to RIGHT and UP. That's FORWARD (X).
          // Let's assume rotation around the axis most perpendicular to the normal and roughly "vertical" or "spanwise".
          // If normal is RIGHT, rotate around UP.
           rotation_axis = PhysicsConfig::BODY_UP; // Check if BODY_UP is correct axis for rudder rotation
     } else {
         // Default guess: Perpendicular to normal and forward?
         rotation_axis = glm::normalize(glm::cross(PhysicsConfig::BODY_FORWARD, base_normal_body));
     }


     // Create rotation quaternion
     glm::quat deflection_rot = glm::angleAxis(deflection_rad, rotation_axis);

     // Rotate the base normal
     return glm::normalize(deflection_rot * base_normal_body);
}


void Wing::applyForces(RigidBody* rigid_body, float max_deflection_angle_deg) const {
    if (!rigid_body || area < 1e-6f) return;

    // 1. Calculate velocity of the wing's center of pressure in world space
    glm::vec3 velocity_world = rigid_body->getPointVelocityWorld(center_of_pressure_body);
    float speed_sq = glm::length2(velocity_world);

    if (speed_sq < 0.1f) return; // Avoid issues at very low speeds
    float speed = glm::sqrt(speed_sq);
    glm::vec3 velocity_dir_world = velocity_world / speed;

    // 2. Determine effective normal vector in body space based on control input
    glm::vec3 effective_normal_body = calculateEffectiveNormal(max_deflection_angle_deg);
    glm::vec3 effective_normal_world = rigid_body->bodyToWorldDir(effective_normal_body);

    // 3. Calculate Angle of Attack (AoA)
    // AoA is the angle between the velocity vector and the wing's chord plane.
    // We approximate this using the angle between the negative velocity direction (relative wind)
    // and the effective normal vector. Alpha = 90deg - Angle(normal, -vel_dir)
    // More direct: Use dot product. sin(AoA) = dot(effective_normal_world, -velocity_dir_world) ?? No.
    // AoA is angle between velocity and chord *plane*. Chord plane is perpendicular to normal.
    // Angle between vector V and plane with normal N is asin(dot(V, N)).
    // Let's use the chord direction. If normal is UP, chord is FORWARD.
    // This is tricky. Blog uses: asin(dot(drag_direction, normal)). Drag_dir = -vel_dir.
    // So, AoA_rad = asin(dot(-velocity_dir_world, effective_normal_world));
    float dot_vn = glm::dot(velocity_dir_world, effective_normal_world);
    // Clamp dot product to [-1, 1] to avoid domain errors with asin
    dot_vn = glm::clamp(dot_vn, -1.0f, 1.0f);
    // Angle between velocity and normal
    float angle_vel_normal_rad = acos(dot_vn);
    // AoA = 90 degrees - angle between velocity and normal
    float aoa_rad = (PhysicsConfig::PI / 2.0f) - angle_vel_normal_rad;
    float aoa_deg = glm::degrees(aoa_rad);

    // Alternative from blog: AoA = asin(dot(-velocity_dir, normal)) - check signs
    // float aoa_rad_blog = std::asin(glm::dot(-velocity_dir_world, effective_normal_world));
    // float aoa_deg_blog = glm::degrees(aoa_rad_blog);
    // Let's stick with the 90-acos version for now.

    // Debugging AoA
    // std::cout << name << " AoA: " << aoa_deg << " deg" << std::endl;


    // 4. Sample Airfoil Coefficients
    auto [lift_coeff, drag_coeff_profile] = airfoil->sample(aoa_deg);

    // Adjust lift coefficient based on flap deflection (blog's simplified method)
    if (flap_ratio > 0.0f) {
        // This formula seems specific, might need adjustment.
        // Assumes control_input directly scales max Cl change.
        float delta_lift_coeff = sqrt(flap_ratio) * airfoil->getMaxCl() * control_input;
        lift_coeff += delta_lift_coeff;
        // Note: Deflecting flaps also increases drag significantly (not fully captured here)
    }


    // 5. Calculate Induced Drag Coefficient
    // Cd_induced = Cl^2 / (pi * AR * e)
    float drag_coeff_induced = 0.0f;
    if (aspect_ratio > 1e-3f && efficiency_factor > 1e-3f) {
        drag_coeff_induced = PhysicsConfig::sq(lift_coeff) / (PhysicsConfig::PI * aspect_ratio * efficiency_factor);
    }
    float drag_coeff_total = drag_coeff_profile + drag_coeff_induced;


    // 6. Calculate Dynamic Pressure and Air Density
    float air_density = PhysicsConfig::get_air_density(rigid_body->position_world.y);
    float dynamic_pressure = 0.5f * air_density * speed_sq; // 0.5 * rho * v^2


    // 7. Calculate Lift and Drag Forces (Magnitude)
    float lift_magnitude = lift_coeff * dynamic_pressure * area;
    float drag_magnitude = drag_coeff_total * dynamic_pressure * area;


    // 8. Determine Lift and Drag Directions (World Space)
    // Drag acts opposite to the velocity vector
    glm::vec3 drag_direction_world = -velocity_dir_world;

    // Lift acts perpendicular to the velocity vector, in the plane containing velocity and the normal vector.
    // Can be found by: cross(cross(velocity_dir, normal), velocity_dir) -> normalized
    // Or simpler (from blog): Lift is perpendicular to drag_dir and the wing's span axis.
    // What is span axis? If normal is UP, span is RIGHT. If normal is RIGHT (rudder), span is UP.
    glm::vec3 span_dir_body; // Wing's span direction in body coords
     if (glm::abs(glm::dot(base_normal_body, PhysicsConfig::BODY_UP)) > 0.9f) {
         span_dir_body = PhysicsConfig::BODY_RIGHT; // Wing/Elevator span
     } else if (glm::abs(glm::dot(base_normal_body, PhysicsConfig::BODY_RIGHT)) > 0.9f) {
         span_dir_body = PhysicsConfig::BODY_UP; // Rudder span (vertical)
     } else {
         span_dir_body = glm::normalize(glm::cross(base_normal_body, PhysicsConfig::BODY_FORWARD)); // Default guess
     }
    glm::vec3 span_dir_world = rigid_body->bodyToWorldDir(span_dir_body);
    glm::vec3 lift_direction_world = glm::normalize(glm::cross(drag_direction_world, span_dir_world));


    // 9. Calculate Final Force Vectors (World Space)
    glm::vec3 lift_force_world = lift_direction_world * lift_magnitude;
    glm::vec3 drag_force_world = drag_direction_world * drag_magnitude;
    glm::vec3 total_aero_force_world = lift_force_world + drag_force_world;

    // 10. Apply Force at the Wing's Center of Pressure
    // Need to convert force back to body space if using addForceAtPointBody,
    // or apply world force at world position of CoP. Let's use the latter.
    glm::vec3 cop_world = rigid_body->bodyToWorldPoint(center_of_pressure_body);
    rigid_body->addForceAtPointWorld(total_aero_force_world, cop_world);

    // --- Debugging Output ---
    // static int frame_count = 0;
    // if (frame_count++ % 60 == 0 && name == "Left Wing") { // Print data for one wing once per second
    //      std::cout << "--- Wing: " << name << " ---" << std::endl;
    //      std::cout << "Speed: " << speed << " m/s, Alt: " << rigid_body->position_world.y << " m, Density: " << air_density << std::endl;
    //      std::cout << "AoA: " << aoa_deg << " deg" << std::endl;
    //      std::cout << "Coeffs (L/Dp/Di/Dt): " << lift_coeff << " / " << drag_coeff_profile << " / " << drag_coeff_induced << " / " << drag_coeff_total << std::endl;
    //      std::cout << "Forces (L/D): " << lift_magnitude << " N / " << drag_magnitude << " N" << std::endl;
    //      std::cout << "Lift Dir: " << lift_direction_world.x << "," << lift_direction_world.y << "," << lift_direction_world.z << std::endl;
    //      std::cout << "Drag Dir: " << drag_direction_world.x << "," << drag_direction_world.y << "," << drag_direction_world.z << std::endl;
    //      std::cout << "Applied Force World: " << total_aero_force_world.x << "," << total_aero_force_world.y << "," << total_aero_force_world.z << std::endl;
    //      std::cout << "Applied At World: " << cop_world.x << "," << cop_world.y << "," << cop_world.z << std::endl;
    //      std::cout << "Body Vel: " << rigid_body->worldToBodyDir(rigid_body->velocity_world).x << "," << rigid_body->worldToBodyDir(rigid_body->velocity_world).y << "," << rigid_body->worldToBodyDir(rigid_body->velocity_world).z << std::endl;
    //      std::cout << "Body AngVel: " << rigid_body->angular_velocity_body.x << "," << rigid_body->angular_velocity_body.y << "," << rigid_body->angular_velocity_body.z << std::endl;
    // }
}

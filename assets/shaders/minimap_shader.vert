#version 330 core
layout (location = 0) in vec2 aPos; // 2D positions

uniform mat4 model; // For positioning/scaling/rotating elements
uniform mat4 projection; // Orthographic projection

void main() {
    gl_Position = projection * model * vec4(aPos.xy, 0.0, 1.0);
}

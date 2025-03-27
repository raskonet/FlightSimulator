#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos; // Received from vertex shader

uniform sampler2D texture1;
uniform vec4 objectColor; // Use for untextured objects if needed
uniform bool useTexture;
uniform vec3 cameraPos; // Needed for fog calculation

// Fog parameters
uniform vec3 fogColor = vec3(0.5, 0.6, 0.7);
uniform float fogDensity = 0.015; // Adjust for desired fog thickness

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    vec4 baseColor = useTexture ? texColor : objectColor;

    // Basic distance fog calculation
    float dist = length(FragPos - cameraPos);
    float fogFactor = exp(-pow(dist * fogDensity, 2.0)); // Exponential fog
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    FragColor = mix(vec4(fogColor, 1.0), baseColor, fogFactor);

    // Discard transparent pixels if necessary (e.g., for alpha masking)
    // if (baseColor.a < 0.1) discard;
}

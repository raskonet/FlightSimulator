#version 330 core
out vec4 FragColor;

// Inputs
in vec3 FragPosWorld;
in vec2 TexCoord;    // <-- Receive UVs
in vec3 NormalWorld; // <-- Receive World Normal

// Uniforms
uniform vec3 u_CameraPos;
uniform vec3 u_SunDirection; // <-- Need sun direction for lighting
uniform sampler2D u_Texture; // <-- Need detail texture

// Simple directional light calculation
vec3 calculateDirLight(vec3 lightDir, vec3 normal, vec3 baseColor)
{
	float ambientStrength = 0.4; // Ambient light level
	float diffuseStrength = 0.8; // Diffuse light level

	vec3 norm = normalize(normal);
	vec3 lightD = normalize(lightDir);

    // Ambient term
    vec3 ambient = ambientStrength * vec3(1.0); // White ambient light

    // Diffuse term
    float diff = max(dot(norm, -lightD), 0.0); // Light direction is TO the light source
    vec3 diffuse = diffuseStrength * diff * vec3(1.0); // White diffuse light

    // Combine and apply to base color
    return (ambient + diffuse) * baseColor;
}

void main()
{
    // Sample the detail texture
    vec3 baseColor = texture(u_Texture, TexCoord).rgb;

    // Calculate lighting using the interpolated world normal
    vec3 litColor = calculateDirLight(u_SunDirection, NormalWorld, baseColor);

    // Basic Fog calculation (same as before)
    float fogMaxdist = 60000.0;
    float fogMindist = 1000.0;
    vec4 fogColor = vec4(0.5, 0.6, 0.7, 1.0); // Sky blueish
    float dist = length(FragPosWorld - u_CameraPos);
    float fogFactor = clamp((fogMaxdist - dist) / (fogMaxdist - fogMindist), 0.0, 1.0);

    // Mix final color with fog
    FragColor = mix(fogColor, vec4(litColor, 1.0), fogFactor);
}

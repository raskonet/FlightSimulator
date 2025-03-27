#version 330 core
layout (location = 0) in vec3 a_Pos; // Input vertex position (flat grid, local space)

// Matrices
uniform mat4 u_View;
uniform mat4 u_Model; // Transforms local grid vertex to world position (XZ plane)
uniform mat4 u_Projection;

// Textures
uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap; // <-- Add Normalmap

// Parameters
uniform float u_TerrainSize;
uniform float u_MaxHeight;

// Outputs
out vec3 FragPosWorld;
out vec2 TexCoord;      // <-- Add TexCoord output
out vec3 NormalWorld;   // <-- Add Normal output (in world space)

// UV Calculation (same as before)
vec2 getWorldXZToUV(vec2 worldXZ) {
    vec2 normalizedPos = worldXZ / u_TerrainSize;
    return normalizedPos + 0.5;
}

// Height Sampling (same as before)
float getHeight(vec2 uv) {
    uv = clamp(uv, 0.0, 1.0);
    // Sample the RED channel of the heightmap
    float heightNormalized = texture(u_Heightmap, uv).r; // <--- Is this sampling correctly?
    // Scale normalized height (0-1) to world height (0-u_MaxHeight)
    return heightNormalized * u_MaxHeight; // <--- Is u_MaxHeight non-zero? Is heightNormalized non-zero?
}
// Normal Sampling and Transformation
vec3 getNormal(vec2 uv) {
    uv = clamp(uv, 0.0, 1.0);
    // Sample normal map; normals are usually stored in [0, 1] range, need to remap to [-1, 1]
    vec3 normal_tangent = texture(u_Normalmap, uv).rgb * 2.0 - 1.0;
    return normalize(normal_tangent); // Return normal in tangent space (relative to surface)
}

void main()
{
    // 1. Calculate initial world position on the XZ plane
    vec3 worldPosXZ = vec3(u_Model * vec4(a_Pos, 1.0));

    // 2. Calculate UV coordinates
    TexCoord = getWorldXZToUV(worldPosXZ.xz); // Pass UVs to fragment shader

  // 3. Sample heightmap using calculated UVs
    float height = getHeight(TexCoord); // <--- Calculate height

    // 4. Create final world position including height
    // THIS IS THE CRITICAL LINE:
    FragPosWorld = vec3(worldPosXZ.x, height, worldPosXZ.z); // <--- Apply the calculated height to Y    // 5. Sample tangent-space normal from map

  vec3 normal_tangent = getNormal(TexCoord);

    // 6. Transform normal from tangent space to world space
    // This requires a TBN matrix (Tangent, Bitangent, Normal)
    // For a flat grid on XZ plane (before height applied), Tangent=X, Bitangent=Z, Normal=Y
    // We need the orientation of the final surface in world space.
    // Approximate TBN from model matrix orientation:
    mat3 normalMatrix = transpose(inverse(mat3(u_Model))); // Matrix to transform normals
    vec3 world_up = vec3(0.0, 1.0, 0.0); // Assuming initial grid normal is world Y up
    // A simpler approach for now: Assume the sampled normal is already world-space oriented
    // OR just pass the tangent space normal and calculate TBN in frag shader (more complex)
    // Let's try assuming the normal map stores world-space normals directly for simplification,
    // OR just use a fixed UP normal for now to test lighting.
    // NormalWorld = normalize(vec3(0.0, 1.0, 0.0)); // Fixed UP normal debug
    // Let's try using the normal map value, assuming it's somewhat world-oriented (might look wrong)
    NormalWorld = normalize(normalMatrix * normal_tangent); // Transform sampled normal by model matrix

    // 7. Calculate final screen position
     gl_Position = u_Projection * u_View * vec4(FragPosWorld, 1.0); 
}

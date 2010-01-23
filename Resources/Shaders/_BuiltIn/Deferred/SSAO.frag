uniform sampler2D   R5_texture0;    // Depth
uniform sampler2D   R5_texture1;    // Normal
uniform sampler2D   R5_texture2;    // Random

uniform vec2 R5_pixelSize;          // 0-1 factor size of the pixel
uniform vec4 R5_clipRange;          // Near/far clipping range
uniform mat4 R5_projectionMatrix;   // Current projection matrix
uniform mat4 R5_inverseProjMatrix;  // Inverse projection matrix

uniform vec2 properties;			// X = focus range, Y = power

//============================================================================================================
// Retrieves depth at the specified texture coordinates
//============================================================================================================

float GetDepth (in vec2 texCoord)
{
    return texture2D(R5_texture0, texCoord).r;
}

//============================================================================================================
// Calculates the view space position from the specified texture coordinates and depth
//============================================================================================================

vec3 GetViewPos (in vec2 texCoord)
{
    vec4 pos = vec4(texCoord.x, texCoord.y, GetDepth(texCoord), 1.0);
    pos.xyz = pos.xyz * 2.0 - 1.0;
    pos = R5_inverseProjMatrix * pos;
    return pos.xyz / pos.w;
}

//============================================================================================================
// Fragment Shader
//============================================================================================================

void main()
{
    const int ssaoSamples = 12;

    // Texture coordinate
    vec2 texCoord = gl_TexCoord[0].xy;

    // View space normal
    vec3 normal = normalize(texture2D(R5_texture1, texCoord).xyz * 2.0 - 1.0);

    // View space position of the pixel
    vec3 pos = GetViewPos(texCoord);

    // Random value sampled from the texture in repeated screen coordinates (32 x 32)
    vec2 modifier = texture2D(R5_texture2, texCoord / R5_pixelSize / 32.0).xy;

    float dist, visibility = 0.0;
    vec4 random, screenPos, viewPos = vec4(1.0);

    for (int i = 0; i < ssaoSamples; i++)
    {
        // Retrieve a new random vector from the texture
        random = texture2D(R5_texture2, modifier);

        // Not much point in normalizing -- no visual difference
        random.xyz = random.xyz * 2.0 - 1.0;

        // Randomize the modifier for the next loop
        modifier += random.xy;

        // Flip the random vector if it's below the plane
        if (dot(random.xyz, normal) < 0.0) random.xyz = -random.xyz;

        // Randomly offset view-space position
        viewPos.xyz = random.xyz * (properties.x * random.w) + pos;

        // Calculate the randomly offset position's screen space coordinates -- second most expensive operation
        screenPos = R5_projectionMatrix * viewPos;

        // Convert screen space coordinates to 0-1 range and get the depth underneath (most expensive operation)
        // This used to be: (screenPos.xy / screenPos.w * 0.5 + 0.5)
        dist = GetDepth(screenPos.xy / (screenPos.w * 2.0) + 0.5);

        // Convert the depth to linear form (note that this value is positive, while viewPos.z is negative)
        dist = R5_clipRange.z / (R5_clipRange.y - dist * R5_clipRange.w);

        // Difference in linear depth relative to the focus range
        dist = (viewPos.z + dist) / properties.x;

        // We want occlusion to fade out if the depth difference becomes too great. We'll use
        // 'properties.x' as the center, and since 'dist' is already relative to it, just add +1.
        dist = abs(dist + 1.0);

        // Distance is currently (0 to X) range -- limit it to (0 to 1) range
        visibility += min(dist, 1.0);
    }

    // Final occlusion factor
    gl_FragColor = vec4(pow(visibility / float(ssaoSamples), properties.y));
}
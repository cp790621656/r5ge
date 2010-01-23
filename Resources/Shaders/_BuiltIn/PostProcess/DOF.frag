uniform sampler2D   R5_texture0;    // Color
uniform sampler2D   R5_texture1;    // Depth
uniform sampler2D   R5_texture2;    // Downsampled and blurred color
uniform sampler2D   R5_texture3;    // Even further downsampled / blurred color
uniform vec4        R5_clipRange;   // Near/far clipping range

// Value 0 = center distance for the depth of field calculations
// Value 1 = how far is the focus edge from the center (past which the original texture begins fading)
// Value 2 = how far from the focus edge first downsample becomes 100%
// Value 3 = how far from the focus edge second downsample becomes 100%
uniform vec4 focusRange;

void main()
{
    // Figure out the distance to this pixel
    float depth = texture2D(R5_texture1, gl_TexCoord[0].xy).r;
    float dist  = R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w);

	// Distance from the focal point
	dist = abs(dist - focusRange.x);

	// Distance from the first edge
	dist -= focusRange.y;
	float factor0 = clamp((focusRange.z - dist) / focusRange.z, 0.0, 1.0);

	// Distance from the second edge
	dist -= focusRange.z;
	float factor1 = clamp((focusRange.w - dist) / focusRange.w, 0.0, 1.0);

    vec3 original    = texture2D(R5_texture0, gl_TexCoord[0].xy).rgb;
    vec3 downsample0 = texture2D(R5_texture2, gl_TexCoord[0].xy).rgb;
    vec3 downsample1 = texture2D(R5_texture3, gl_TexCoord[0].xy).rgb;

    vec3 final = mix( mix(downsample1, downsample0, factor1 * factor1), original, factor0);
    gl_FragColor = vec4(final, 1.0);
}
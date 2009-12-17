//============================================================================================================
// Darkens pixels based on distance
//============================================================================================================

uniform sampler2D   R5_texture0;	// Diffuse texture (RGBA)
uniform sampler2D   R5_texture1;	// Depth texture

uniform vec4 R5_clipRange;

void main()
{
	vec4 colorMap = texture2D(R5_texture0, gl_TexCoord[0].xy);
	vec4 depthMap = texture2D(R5_texture1, gl_TexCoord[0].xy);

    float tint = (R5_clipRange.z / (R5_clipRange.y - depthMap.r * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;

	tint -= 0.333;
	tint /= 0.667;

	tint = clamp(tint, 0.0, 1.0);
	tint = 1.0 - tint;
	tint *= tint;

    gl_FragColor = vec4(colorMap.rgb * tint, colorMap.a);
}
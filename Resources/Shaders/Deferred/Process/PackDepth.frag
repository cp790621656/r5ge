uniform sampler2D   R5_texture0;
uniform vec4        R5_clipRange;

//============================================================================================================
// Pack float as RGBA
//============================================================================================================

vec4 PackFloat (in float val)
{
	const vec4 multiplier = vec4(16777216.0, 65535.0, 256.0, 1.0);
	const vec4 bitMask = vec4(0.0, 0.00390625, 0.00390625, 0.00390625);
	vec4 ret = fract(multiplier * val);
	return ret - ret.xxyz * bitMask;
}

//============================================================================================================
// Unpack float from RGBA
//============================================================================================================

//float UnpackFloat (in vec4 val)
//{
//	const vec4 multiplier = vec4(1.0 / 16777216.0, 1.0 / 65535.0, 1.0 / 256.0, 1.0);
//	return dot(val, multiplier);
//}

//============================================================================================================
// Main function -- read depth and encode it as a linear distance in RGBA format
//============================================================================================================

void main()
{
    float depth = texture2D(R5_texture0, gl_TexCoord[0].xy).r;
    float val = (R5_clipRange.x - R5_clipRange.z / (depth * R5_clipRange.w - R5_clipRange.y)) / R5_clipRange.w;
    gl_FragColor = PackFloat(val);
}
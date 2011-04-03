#version 150
uniform sampler2DMS R5_texture0;
uniform vec2 R5_pixelSize;

out vec4 FinalColor;

void main()
{
	FinalColor = texelFetch(R5_texture0, ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y)), 0);
}
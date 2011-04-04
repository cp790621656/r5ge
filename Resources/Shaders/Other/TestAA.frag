#version 150
uniform sampler2DMS R5_texture0;
uniform vec2 R5_pixelSize;

out vec4 FinalColor;

void main()
{
	ivec2 itc = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));

	vec4 a = texelFetch(R5_texture0, itc, 0);
	a += texelFetch(R5_texture0, itc, 1);
	a += texelFetch(R5_texture0, itc, 2);
	a += texelFetch(R5_texture0, itc, 3);
	a += texelFetch(R5_texture0, itc, 4);
	a += texelFetch(R5_texture0, itc, 5);
	a += texelFetch(R5_texture0, itc, 6);
	a += texelFetch(R5_texture0, itc, 7);

	FinalColor = a * 0.125;
}
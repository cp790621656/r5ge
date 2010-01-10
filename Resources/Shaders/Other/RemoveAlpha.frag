uniform sampler2D R5_texture0;
uniform vec2 R5_pixelSize;

void main()
{
	vec4 color = texture2D(R5_texture0, gl_FragCoord.xy * R5_pixelSize);
	if (color.a == 0.0) discard;
	color.a = 1.0;
	gl_FragColor = color;
}
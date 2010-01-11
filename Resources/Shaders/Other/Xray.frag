uniform sampler2D   R5_texture0;
uniform vec2        R5_pixelSize;

void main()
{
    gl_FragColor = texture2D(R5_texture0, gl_FragCoord.xy * R5_pixelSize);
}
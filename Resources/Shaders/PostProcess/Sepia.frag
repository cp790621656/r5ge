uniform sampler2D   R5_texture0;

void main()
{
    vec4 color = texture2D(R5_texture0, gl_TexCoord[0].xy);
    gl_FragColor = vec4(color.r * 0.393 + color.g * 0.769 + color.b * 0.189,
                        color.r * 0.349 + color.g * 0.686 + color.b * 0.168,
                        color.r * 0.272 + color.g * 0.534 + color.b * 0.131,
                        color.a);
}
uniform sampler2D   R5_texture0;
uniform sampler2D   R5_texture1;
uniform sampler2D   R5_texture2;

void main()
{
    gl_FragColor = (texture2D(R5_texture0, gl_TexCoord[0].xy) +
                    texture2D(R5_texture1, gl_TexCoord[0].xy) +
                    texture2D(R5_texture2, gl_TexCoord[0].xy)) * 0.333334;
}
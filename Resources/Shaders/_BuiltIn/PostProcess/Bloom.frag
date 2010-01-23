uniform sampler2D   R5_texture0;
uniform sampler2D   R5_texture1;
uniform sampler2D   R5_texture2;
uniform sampler2D   R5_texture3;

void main()
{
    vec4 original   =  texture2D(R5_texture0, gl_TexCoord[0].xy);
    vec4 downsample = (texture2D(R5_texture1, gl_TexCoord[0].xy) +
                       texture2D(R5_texture2, gl_TexCoord[0].xy) +
                       texture2D(R5_texture3, gl_TexCoord[0].xy));

    gl_FragColor = original + downsample;
}
uniform sampler2D   R5_texture0;    // Material Diffuse (RGBA)
uniform sampler2D   R5_texture1;    // Material Specular (RGB) + Glow (A)
uniform sampler2D   R5_texture2;    // Light Diffuse (RGB)
uniform sampler2D   R5_texture3;    // Light Specular (RGB)

void main()
{
    vec4 matDiff    = texture2D(R5_texture0, gl_TexCoord[0].xy);
    vec4 matSpec    = texture2D(R5_texture1, gl_TexCoord[0].xy);
    vec4 lightDiff  = texture2D(R5_texture2, gl_TexCoord[0].xy);
    vec4 lightSpec  = texture2D(R5_texture3, gl_TexCoord[0].xy);

    gl_FragColor = vec4(matDiff.rgb * matSpec.a +
                        matDiff.rgb * lightDiff.rgb * (1.0 - matSpec.a) +
                        matSpec.rgb * lightSpec.rgb, matDiff.a);
}
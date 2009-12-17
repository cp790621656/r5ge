//============================================================================================================
// Fragment Shader used to bake ambient occlusion or light maps
//============================================================================================================

uniform sampler2D   R5_texture0;	// Diffuse texture (RGBA)
uniform sampler2D   R5_texture1;	// AO / lightmap texture (Alpha)

void main()
{
    vec4 color 		= texture2D(R5_texture0, gl_TexCoord[0].xy);
    vec4 ao 		= texture2D(R5_texture1, gl_TexCoord[0].xy);
    gl_FragColor 	= vec4(color.rgb * ao.r, color.a);
}
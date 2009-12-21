//============================================================================================================
// Fragment Shader used to render objects into the screen-space normal map texture
//============================================================================================================

uniform sampler2D R5_texture0;	// Diffuse map (RGBA)
uniform sampler2D R5_texture1;	// Normal map (RGB)

varying vec2 _texCoord;
varying vec3 _normal, _tangent;

void main()
{
	vec4 diffuseMap	= texture2D(R5_texture0, _texCoord);
	vec4 normalMap 	= texture2D(R5_texture1, _texCoord);
    vec3 tangent  	= normalize(_tangent);
    vec3 normal   	= normalize(_normal);
    mat3 TBN      	= mat3(tangent, cross(normal, tangent), normal);
    normal        	= normalize(TBN * (normalMap.xyz * 2.0 - 1.0));
    gl_FragColor 	= vec4(normal * 0.5 + 0.5, min(1.0, diffuseMap.a));
}
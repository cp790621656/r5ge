// R5_INCLUDE Shaders/Deferred/D_N.vert

uniform sampler2D	R5_texture0;	// Diffuse (RGBA)
uniform sampler2D	R5_texture1;	// Normal (RGB) + Specularity (A)

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _tangent;

void main()
{
	// Diffuse map
	vec4 normalMap = texture2D(R5_texture1, _texCoord);

	// Normal map
	vec3 tangent = normalize(_tangent);
	vec3 normal  = normalize(_normal);
	mat3 TBN	 = mat3(tangent, cross(normal, tangent), normal);
	normal		 = TBN * normalize(normalMap.rgb * 2.0 - 1.0);

	// Encode the values
	gl_FragData[0] = gl_FrontMaterial.diffuse * texture2D(R5_texture0, _texCoord) * gl_Color;
	gl_FragData[1] = vec4(
		R5_MATERIAL_SPECULARITY * normalMap.a,
		R5_MATERIAL_SPECULAR_HUE,
		R5_MATERIAL_GLOW, 1.0);
	gl_FragData[2] = vec4(normalize(normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}

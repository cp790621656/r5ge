uniform sampler2D R5_texture0;

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _tangent;

void main()
{
	vec4 diffuse = gl_FrontMaterial.diffuse * gl_Color;

	vec4 maps = vec4(
		R5_MATERIAL_SPECULARITY,
		R5_MATERIAL_SPECULAR_HUE,
		R5_MATERIAL_GLOW,
		1.0);

	vec3 tangent  	= normalize(_tangent);
	vec4 normal   	= vec4(normalize(_normal), R5_MATERIAL_SHININESS);
	mat3 TBN	  	= mat3(tangent, cross(normal.xyz, tangent), normal);
	normal.xyz	  	= normalize(TBN * normalize(texture2D(R5_texture0, _texCoord).xyz * 2.0 - 1.0));

	// R5_FRAGMENT_OUTPUT diffuse maps normal
}

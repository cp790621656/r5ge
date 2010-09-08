uniform sampler2D R5_texture0;

varying vec2 _texCoord;
varying vec3 _normal, _tangent;

//============================================================================================================
// Fragment Shader
//============================================================================================================

void main()
{
	vec3 tangent  	= normalize(_tangent);
	vec3 normal   	= normalize(_normal);
	mat3 TBN	  	= mat3(tangent, cross(normal, tangent), normal);
	normal		  	= normalize(TBN * normalize(texture2D(R5_texture0, _texCoord).xyz * 2.0 - 1.0));

	vec4 diffuse 	= gl_FrontMaterial.diffuse;
	vec4 maps	 	= vec4(	R5_MATERIAL_SPECULARITY,
							R5_MATERIAL_SPECULAR_HUE,
							R5_MATERIAL_GLOW, 1.0 );
	float shininess	= R5_MATERIAL_SHININESS;

	// R5_IMPLEMENT_LIGHTING normal diffuse maps shininess
}

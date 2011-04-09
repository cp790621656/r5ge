uniform sampler2D R5_texture0;
varying vec2 _texCoord;
varying vec3 _normal;

void main()
{
	vec4 diffuse = gl_FrontMaterial.diffuse * gl_Color;

	vec4 maps = vec4(
		R5_MATERIAL_SPECULARITY,
		R5_MATERIAL_SPECULAR_HUE,
		R5_MATERIAL_GLOW,
		R5_MATERIAL_OCCLUSION * texture2D(R5_texture0, _texCoord).a);

	vec4 normal = vec4(normalize(_normal), R5_MATERIAL_SHININESS);

	// R5_FRAGMENT_OUTPUT diffuse maps normal
}
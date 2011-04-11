varying vec3 _normal;

void main()
{
	vec4 diffuse = gl_FrontMaterial.diffuse * gl_Color;
	vec4 maps 	 = vec4(R5_MATERIAL_SPECULARITY, R5_MATERIAL_SPECULAR_HUE, R5_MATERIAL_GLOW, 1.0);
	vec4 normal  = vec4(normalize(_normal), R5_MATERIAL_SHININESS);

	// R5_FRAGMENT_OUTPUT diffuse maps normal
}

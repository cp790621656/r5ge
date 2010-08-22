varying vec3 _normal;

void main()
{
	// Diffuse RGBA
	gl_FragData[0] = gl_FrontMaterial.diffuse * gl_Color;

	gl_FragData[1] = vec4(
		R5_MATERIAL_SPECULARITY,
		R5_MATERIAL_SPECULAR_HUE,
		R5_MATERIAL_GLOW,
		1.0);

	// Normal (RGB), Shininess (A)
	gl_FragData[2] = vec4(normalize(_normal) * 0.5 + 0.5, R5_MATERIAL_SHININESS);
}
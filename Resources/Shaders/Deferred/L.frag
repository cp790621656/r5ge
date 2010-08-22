uniform sampler2D R5_texture0;

varying vec2 _texCoord0;
varying vec3 _normal;

void main()
{
	float ao = texture2D(R5_texture0, _texCoord0).a;

	gl_FragData[0] = gl_FrontMaterial.diffuse * gl_Color;

	gl_FragData[1] = vec4(
		R5_MATERIAL_SPECULARITY,
		R5_MATERIAL_SPECULAR_HUE,
		R5_MATERIAL_GLOW,
		R5_MATERIAL_OCCLUSION * ao);

	gl_FragData[2] = vec4(normalize(_normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}

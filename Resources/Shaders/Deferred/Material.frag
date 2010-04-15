varying vec3 _normal;

void main()
{
	gl_FragData[0] = gl_FrontMaterial.diffuse * gl_Color;
	gl_FragData[1] = vec4(gl_FrontMaterial.specular.rgb, gl_FrontMaterial.emission.a);
	gl_FragData[2] = vec4(normalize(_normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}
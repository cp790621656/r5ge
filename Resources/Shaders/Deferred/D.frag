uniform sampler2D	R5_texture0;

varying vec2 _texCoord;
varying vec3 _normal;

void main()
{
	gl_FragData[0] = gl_FrontMaterial.diffuse * texture2D(R5_texture0, _texCoord) * gl_Color;
	gl_FragData[1] = vec4(gl_FrontMaterial.specular.rgb, gl_FrontMaterial.emission.a);
	gl_FragData[2] = vec4(normalize(_normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}

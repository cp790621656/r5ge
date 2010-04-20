uniform sampler2D R5_texture0;
uniform sampler2D R5_texture1;

varying vec2 _texCoord0;
varying vec2 _texCoord1;
varying vec3 _normal;

void main()
{
	vec4 diffuse   = texture2D(R5_texture0, _texCoord0);
	vec4 lightmap  = texture2D(R5_texture1, _texCoord1);
	
	diffuse.rgb *= lightmap.rgb;

	gl_FragData[0] = gl_FrontMaterial.diffuse * diffuse * gl_Color;
	gl_FragData[1] = vec4(gl_FrontMaterial.specular.rgb, gl_FrontMaterial.emission.a);
	gl_FragData[2] = vec4(normalize(_normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}

uniform sampler2D	R5_texture0;	// Diffuse (RGB)
uniform sampler2D	R5_texture1;	// Specular (R), Reflection (G), Glow (B)
uniform samplerCube R5_texture2; 	// Environment map (RGB)

uniform mat3 R5_inverseViewRotationMatrix;

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _eyeDir;

void main()
{
	vec3 diffuse 	= texture2D(R5_texture0, _texCoord).rgb;
	vec3 maps 		= texture2D(R5_texture1, _texCoord).rgb;
	vec3 normal 	= normalize(_normal);

	// Reflect the directional vector then transform the result into world space
	vec3 eyeDir		= normalize(_eyeDir);
	vec3 incident	= R5_inverseViewRotationMatrix * reflect(eyeDir, normal);

	// Start with the diffuse color
	vec3 color = (gl_FrontMaterial.diffuse.rgb * diffuse) * max(1.0, maps.b * 4.0);

	// Reflected color is just the diffuse color modulated by the reflection map
	vec3 reflectedColor = color * textureCube(R5_texture2, incident).a;

	// Mix the diffuse color with the reflected color
	color = mix(color, reflectedColor, maps.g);

	// Encode the values
	gl_FragData[0] = vec4(color, gl_FrontMaterial.diffuse.a);
	gl_FragData[1] = vec4(gl_FrontMaterial.specular.rgb * maps.r, gl_FrontMaterial.emission.a + maps.b);
	gl_FragData[2] = vec4(normal * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}

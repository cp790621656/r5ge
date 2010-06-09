uniform sampler2D   R5_texture0;    // Diffuse (RGB) + Specular (A)
uniform sampler2D   R5_texture1;    // Glow (A)
uniform samplerCube R5_texture2; 	// Environment map (RGB)

uniform mat3 R5_inverseViewRotationMatrix;

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _eyeDir;

void main()
{
	vec4 diffuseMap = texture2D(R5_texture0, _texCoord);
	vec3 normal = normalize(_normal);

	float glow		 	 = texture2D(R5_texture1, _texCoord).r;
    float specularity	 = min(1.0, diffuseMap.a * 3.0); 			// TODO: Fix the specular map
    float reflectiveness = max(0.0, min(0.35, specularity) - glow);	// TODO: Add a reflection map

    // Reflect the directional vector then transform the result into world space
    vec3 eyeDir		= normalize(_eyeDir);
    vec3 incident	= R5_inverseViewRotationMatrix * reflect(eyeDir, normal);

	// Start with the diffuse color
    vec3 color = (gl_FrontMaterial.diffuse.rgb * diffuseMap.rgb) * max(1.0, glow * 4.0);

    // Mix the diffuse color with the reflected color
    color = mix(color, textureCube(R5_texture2, incident).rgb, reflectiveness);

    // Encode the values
    gl_FragData[0] = vec4(color, gl_FrontMaterial.diffuse.a);
    gl_FragData[1] = vec4(gl_FrontMaterial.specular.rgb * specularity, gl_FrontMaterial.emission.a + glow);
    gl_FragData[2] = vec4(normal * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}
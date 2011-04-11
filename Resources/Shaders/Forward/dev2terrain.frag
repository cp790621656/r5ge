uniform sampler2D	R5_texture0; // Normal map (RGB) and height (A)
uniform sampler2D	R5_texture1; // Gradient mix texture
uniform sampler2D	R5_texture2; // Rocky texture
uniform sampler2D	R5_texture3; // Rocky normal map
uniform sampler2D	R5_texture4; // Smooth texture
uniform sampler2D	R5_texture5; // Smooth normal map

varying float _fogFactor;
varying vec2 _texCoord, _mapCoord;
varying vec3 _eyeDir, _tangent;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
	vec4 normalMap	= texture2D(R5_texture0, _mapCoord);
	vec4 colorMap	= texture2D(R5_texture1, _mapCoord);
	vec4 tex0		= texture2D(R5_texture2, _texCoord);
	vec4 tex1		= texture2D(R5_texture3, _texCoord);
	vec4 tex2		= texture2D(R5_texture4, _texCoord);
	vec4 tex3		= texture2D(R5_texture5, _texCoord);
	vec3 eyeDir		= normalize(_eyeDir);
	vec3 tangent	= normalize(_tangent);
	vec3 normal		= normalMap.xyz * 2.0 - 1.0;
	normal			= normalize(gl_NormalMatrix * normal);
	vec3 binormal	= cross(normal, tangent);

	// Realign the tangent
	tangent = cross(binormal, normal);

	// Create the TBN matrix
	mat3 TBN = mat3(tangent, binormal, normal);

	// Update the normal map
	normalMap = mix(tex1, tex3, colorMap.r);

    // Transform the updated normal by the TBN matrix
	normal = normalMap.rgb * 2.0 - 1.0;
	normal = normalize(TBN * normal);

	// Tint the textures
	tex0.rgb *= colorMap.g;
	tex2.rgb *= colorMap.b;

	// Diffuse color
	vec4 color = mix(tex0, tex2, colorMap.r);

	// Directional light
	vec3  lightDir 			= normalize(gl_LightSource[0].position.xyz);
    vec3  reflected 		= reflect(lightDir, normal);
	float diffuseFactor     = max( 0.0, dot(normal, lightDir) );
	float reflectiveFactor  = max( 0.0, dot(reflected, eyeDir) );
	float specularFactor    = pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess ) * normalMap.a;

	// Modulate by the light's diffuse and add the specular component
	color.rgb  *= gl_FrontLightProduct[0].ambient.rgb +
				  gl_FrontLightProduct[0].diffuse.rgb * diffuseFactor;
	color.rgb  += gl_FrontLightProduct[0].specular.rgb * specularFactor;

	// Adjust by the fog
	color.rgb = mix(color.rgb, gl_Fog.color.rgb, _fogFactor);

	// Final color
	gl_FragColor = color;
}

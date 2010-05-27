uniform sampler2D R5_texture0;
uniform vec2 R5_pixelSize;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal;
varying float _fogFactor;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
	vec3 normal = normalize(_normal);
	vec3 eyeDir = normalize(_eyeDir);

	float diffuseFactor, reflectiveFactor, specularFactor;
	vec3 lightDir, reflected;

	// Directional light
	lightDir  = normalize(gl_LightSource[0].position.xyz);
	reflected = reflect(lightDir, normal);

	// Calculate the diffuse and specular contributions
	diffuseFactor	  	= max( 0.0, dot(normal, lightDir) );
	reflectiveFactor  	= max( 0.0, dot(reflected, eyeDir) );
	specularFactor	  	= pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess );

	// Apply the shadow
	float shadowFactor 	= texture2D(R5_texture0, gl_FragCoord.xy * R5_pixelSize).a;
	diffuseFactor  		= min(diffuseFactor, shadowFactor);
	specularFactor 		= min(diffuseFactor, specularFactor);

	// Diffuse color
	vec3 diffuse = gl_FrontLightProduct[0].ambient.rgb + gl_FrontLightProduct[0].diffuse.rgb * diffuseFactor;

	// Specular color
	vec3 specular = gl_FrontLightProduct[0].specular.rgb * specularFactor;

	// Combine the colors
	vec4 color  = vec4(diffuse.r, diffuse.g, diffuse.b, gl_FrontMaterial.diffuse.a);
	color.rgb	= mix(color.rgb, gl_FrontMaterial.emission.rgb, gl_FrontMaterial.emission.a);
	color.rgb  += specular;
	color.rgb	= mix(color.rgb, gl_Fog.color.rgb, _fogFactor);

	gl_FragColor = color;
}

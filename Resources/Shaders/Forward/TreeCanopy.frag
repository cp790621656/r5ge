#version 110

uniform sampler2D R5_texture0;
uniform sampler2D R5_texture1;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal, _tangent;
varying float _fogFactor;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
	vec4 texture1 	= texture2D(R5_texture1, _texCoord);
	vec3 tangent  	= normalize(_tangent);
	vec3 normal   	= normalize(_normal);
	mat3 TBN	  	= mat3(tangent, cross(normal, tangent), normal);
	normal			= texture1.xyz * 2.0 - 1.0;
	normal		  	= normalize(TBN * normal);
	vec3 eyeDir   	= normalize(_eyeDir);
	vec4 color	  	= vec4(0.0, 0.0, 0.0, gl_FrontMaterial.diffuse.a);
	vec4 specular 	= vec4(0.0, 0.0, 0.0, 0.0);

	float diffuseFactor, reflectiveFactor, specularFactor;
	vec3 lightDir, reflected;

	// Directional light
	lightDir  = normalize(gl_LightSource[0].position.xyz);
	reflected = reflect(lightDir, normal);

	diffuseFactor	  = max( 0.0, dot(normal, lightDir) );
	reflectiveFactor  = max( 0.0, dot(reflected, eyeDir) );
	specularFactor	  = pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess );

	color.rgb  += gl_FrontLightProduct[0].ambient.rgb;
	color.rgb  += gl_FrontLightProduct[0].diffuse.rgb * diffuseFactor;
	specular   += gl_FrontLightProduct[0].specular * specularFactor;

	color	  *= texture2D(R5_texture0, _texCoord);
	color.rgb += specular.rgb * texture1.a;
	color.rgb  = mix(color.rgb, gl_Fog.color.rgb, _fogFactor);
	color.a    = min(color.a, 1.0);

	gl_FragColor = color;
}

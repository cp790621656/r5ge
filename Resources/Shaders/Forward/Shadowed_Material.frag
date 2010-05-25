#version 110

uniform sampler2DShadow R5_texture0;

varying vec4 _inShadowPos;
varying vec2 _texCoord;
varying vec3 _eyeDir, _normal;
varying float _fogFactor;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
	vec3 normal   = normalize(_normal);
	vec3 eyeDir   = normalize(_eyeDir);
	vec4 color	  = vec4(0.0, 0.0, 0.0, gl_FrontMaterial.diffuse.a);
	vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);

	float diffuseFactor, reflectiveFactor, specularFactor;
	vec3 lightDir, reflected;

	// Directional light
	lightDir  = normalize(gl_LightSource[0].position.xyz);
	reflected = reflect(lightDir, normal);

	diffuseFactor	  = max( 0.0, dot(normal, lightDir) );
	reflectiveFactor  = max( 0.0, dot(reflected, eyeDir) );
	specularFactor	  = pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess );

	// Shadow mapping
	{
	    // Normalized 0-1 range coordinate for shadow sampling
		vec3  coordPos = _inShadowPos.xyz / _inShadowPos.w;
		float minCoord = min(coordPos.x, coordPos.y);
		float maxCoord = max(coordPos.x, coordPos.y);
	
		// We need to only sample the shadow if this coordinate lies within our shadow texture
		if (minCoord > 0.0 && maxCoord < 1.0)
		{
			float shadowFactor = shadow2D(R5_texture0, coordPos).r;
	
			// PCF
			{
				// 30 degree rotated 2x2 kernel
		        float offsetT = 1.0 / 512.0;
		        float offsetX = 0.866 * offsetT;
		        float offsetY = 0.5 * offsetT;

				shadowFactor += shadow2D(R5_texture0, coordPos + vec3(-offsetX,  offsetY, 0.0)).r;
				shadowFactor += shadow2D(R5_texture0, coordPos + vec3(-offsetX, -offsetY, 0.0)).r;
				shadowFactor += shadow2D(R5_texture0, coordPos + vec3( offsetX, -offsetY, 0.0)).r;
				shadowFactor += shadow2D(R5_texture0, coordPos + vec3( offsetX,  offsetY, 0.0)).r;
				shadowFactor *= 0.2;
			}

			// Shadow simply drops the diffuse light's intensity
			diffuseFactor  = min(diffuseFactor, shadowFactor);
			specularFactor = min(specularFactor, shadowFactor);
		}
		else
		{
			// The coordinate lies outside the shadow
			diffuseFactor  = 0.0;
			specularFactor = 0.0;
		}
	}

	color.rgb  += gl_FrontLightProduct[0].ambient.rgb;
	color.rgb  += gl_FrontLightProduct[0].diffuse.rgb * diffuseFactor;
	specular   += gl_FrontLightProduct[0].specular * specularFactor;

	color.rgb	= mix(color.rgb, gl_FrontMaterial.emission.rgb, gl_FrontMaterial.emission.a);
	color.rgb  += specular.rgb;
	color.rgb	= mix(color.rgb, gl_Fog.color.rgb, _fogFactor);
	color.a 	= min(color.a, 1.0);

	gl_FragColor = color;
}

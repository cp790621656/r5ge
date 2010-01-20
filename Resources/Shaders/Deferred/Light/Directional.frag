uniform sampler2D	R5_texture0;	// Depth
uniform sampler2D	R5_texture1;	// View space normal and material shininess

uniform mat4 R5_inverseProjMatrix;	// Inverse projection matrix
uniform vec2 R5_pixelSize;			// (1 / width, 1 / height)

void main()
{
	// Figure out the pixel's 0-1 range screen space position
	vec3 view = vec3(gl_FragCoord.x * R5_pixelSize.x,
					 gl_FragCoord.y * R5_pixelSize.y, 0.0);

	vec4 normalMap = texture2D(R5_texture1, view.xy);

	// Using the XY portion of screen position, sample the textures
	float depth 	= texture2D(R5_texture0, view.xy).r;
	vec3  normal	= normalize(normalMap.rgb * 2.0 - 1.0);
	float shininess = 4.0 + (normalMap.a * normalMap.a) * 250.0;

	// Transform from screen space position to view space direction
	view.z = depth;
	view = (R5_inverseProjMatrix * vec4(view * 2.0 - 1.0, 1.0)).xyz;
	view = normalize(view);

	vec4 diffuse  = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);

	// R5_FOR_EACH_LIGHT
	{
		vec3 light = normalize(gl_LightSource[0].position.xyz);

		// Calculate contribution factors (view is flipped, so subtract instead of add here)
		float diffuseFactor 	= max(0.0, dot(light, normal));
		float reflectiveFactor 	= max(0.0, dot(normal, normalize(light - view)));
		float specularFactor 	= pow(reflectiveFactor, shininess);

   		// Taking diffuse into account avoids the "halo" artifact. pow(3) smooths it out.
		float temp = 1.0 - diffuseFactor;
		specularFactor *= 1.0 - temp * temp * temp;

		// Diffuse and ambient component
		diffuse += gl_LightSource[0].ambient + gl_LightSource[0].diffuse * diffuseFactor;

		// Specular component
		specular += gl_LightSource[0].specular * specularFactor;
	}
	
	gl_FragData[0] = diffuse;
	gl_FragData[1] = specular;
}

uniform sampler2D	R5_texture0;	// Depth
uniform sampler2D	R5_texture1;	// View space normal and shininess

uniform mat4 R5_inverseProjMatrix;	// Inverse projection matrix
uniform vec2 R5_pixelSize;			// (1 / width, 1 / height)

//============================================================================================================
// Gets the view space position at the specified texture coordinates
//============================================================================================================

vec3 GetViewPos (in vec2 texCoord)
{
	float depth = texture2D(R5_texture0, texCoord).r;
	vec4 view = vec4(texCoord.x, texCoord.y, depth, 1.0);
	view.xyz = view.xyz * 2.0 - 1.0;
	view = R5_inverseProjMatrix * view;
	return view.xyz / view.w;
}

//============================================================================================================
// Fragment Shader
//============================================================================================================

void main()
{
	// Figure out the pixel's texture coordinates
	vec2 texCoord = gl_FragCoord.xy * R5_pixelSize;

	// Get the depth at this pixel
	vec3 view = GetViewPos(texCoord);

	// Determine this pixel's distance to the light source
	vec3 light = gl_LightSource[0].position.xyz - view;
	float dist = length(light);

	// If the pixel is out of range, discard it
	float range = gl_LightSource[0].constantAttenuation;
	if (dist > range) discard;

	// Get the view space normal
  	vec4  normalMap = texture2D(R5_texture1, texCoord);
	vec3  normal	= normalize(normalMap.rgb * 2.0 - 1.0);
	float shininess = 4.0 + (normalMap.a * normalMap.a) * 250.0;

	// Light's attenuation is stored in the constant attenuation parameter
	float atten = 1.0 - dist / range;

	// Light's power is stored in linear attenuation parameter
	atten = pow(atten, gl_LightSource[0].linearAttenuation);

	// Normalize our view and light vectors
	view  = normalize(view);
	light = normalize(light);

	// Calculate contribution factors (view is flipped, so subtract instead of add here)
	float diffuseFactor 	= max(0.0, dot(light, normal));
	float reflectiveFactor 	= max(0.0, dot(normal, normalize(light - view)));
	float specularFactor 	= pow(reflectiveFactor, shininess);

   	// Taking diffuse into account avoids the "halo" artifact. pow(3) smooths it out.
	float temp = 1.0 - diffuseFactor;
	specularFactor *= 1.0 - temp * temp * temp;

	// Diffuse and ambient component
	gl_FragData[0] = gl_LightSource[0].ambient * atten + gl_LightSource[0].diffuse * (diffuseFactor * atten);

	// Specular component
	gl_FragData[1] = gl_LightSource[0].specular * (specularFactor * atten);
}

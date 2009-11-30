uniform sampler2D   R5_texture0;    // Depth
uniform sampler2D   R5_texture1;    // View space normal and shininess

uniform mat4 R5_inverseProjMatrix;  // Inverse projection matrix
uniform vec2 R5_pixelSize;          // (1 / width, 1 / height)

// Gets the view space position at the specified texture coordinates
vec3 GetViewPos (in vec2 texCoord)
{
	float depth = texture2D(R5_texture0, texCoord).r;
	vec4 pos = vec4(texCoord.x, texCoord.y, depth, 1.0);
    pos.xyz = pos.xyz * 2.0 - 1.0;
    pos = R5_inverseProjMatrix * pos;
    return pos.xyz / pos.w;
}

void main()
{
    // Figure out the pixel's texture coordinates
    vec2 texCoord = gl_FragCoord.xy * R5_pixelSize;

	// Get the depth at this pixel
    vec3 pos = GetViewPos(texCoord);

	// Determine this pixel's distance to the light source
    vec3 incident = gl_LightSource[0].position.xyz - pos;
    float dist = length(incident);

	// If the pixel is out of range, discard it
    float range = gl_LightSource[0].constantAttenuation;
    if (dist > range) discard;

	// Get the view space normal
  	vec4  normalMap = texture2D(R5_texture1, texCoord);
	vec3  normal    = normalize(normalMap.rgb * 2.0 - 1.0);
    float shininess = 4.0 + (normalMap.a * normalMap.a) * 60.0;

    // Light's attenuation is stored in the constant attenuation parameter
    float atten = 1.0 - dist / range;

    // Light's power is stored in linear attenuation parameter
    atten = pow(atten, gl_LightSource[0].linearAttenuation);

    // Incident vector should be normalized
    incident = normalize(incident);

    // Calculate contribution factors
    float diffuseFactor     = max( 0.0, dot(incident, normal) );
    float reflectiveFactor  = max( 0.0, dot(reflect(incident, normal), normalize(pos)) );
    float specularFactor    = pow( reflectiveFactor, shininess );

    // Diffuse and ambient component
    gl_FragData[0] = gl_LightSource[0].ambient * atten + gl_LightSource[0].diffuse * (diffuseFactor * atten);

    // Specular component
    gl_FragData[1] = gl_LightSource[0].specular * (specularFactor * atten);
}
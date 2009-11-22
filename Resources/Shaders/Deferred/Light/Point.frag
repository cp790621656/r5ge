uniform sampler2D   R5_texture0;    // Depth
uniform sampler2D   R5_texture1;    // View space normal and shininess

uniform mat4 R5_inverseProjMatrix;  // Inverse projection matrix
uniform vec2 R5_pixelSize;          // (1 / width, 1 / height)

void main()
{
    // Figure out the pixel's 0-1 range screen space position
    vec3 pos = vec3(gl_FragCoord.x * R5_pixelSize.x,
                    gl_FragCoord.y * R5_pixelSize.y, 0.0);

	vec4 normalMap = texture2D(R5_texture1, pos.xy);

    // Using the XY portion of screen position, sample the textures
    float depth     = texture2D(R5_texture0, pos.xy).r;
    vec3  normal    = normalize(normalMap.rgb * 2.0 - 1.0);
    float shininess = 4.0 + (normalMap.a * normalMap.a) * 60.0;

    // Transform the position from screen space to view space
    pos.z = depth;
    vec4 clip = R5_inverseProjMatrix * vec4(pos * 2.0 - 1.0, 1.0);
    pos = clip.xyz / clip.w;

    // Light's attenuation is stored in the constant attenuation parameter
    vec3 incident = gl_LightSource[0].position.xyz - pos;
    float dist    = length(incident);
    float atten   = 1.0 - clamp(dist / gl_LightSource[0].constantAttenuation, 0.0, 1.0);

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
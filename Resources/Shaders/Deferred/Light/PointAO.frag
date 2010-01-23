uniform sampler2D   R5_texture0;    // Depth
uniform sampler2D   R5_texture1;    // View space normal and shininess
uniform sampler2D   R5_texture2;    // Ambient Lightmap

uniform mat4 R5_inverseProjMatrix;  // Inverse projection matrix
uniform vec2 R5_pixelSize;          // (1 / width, 1 / height)

void main()
{
    // Figure out the pixel's 0-1 range screen space position
    vec3 view = vec3(gl_FragCoord.x * R5_pixelSize.x,
                    gl_FragCoord.y * R5_pixelSize.y, 0.0);

	vec4 normalMap = texture2D(R5_texture1, view.xy);

    // Using the XY portion of screen position, sample the textures
    float depth     = texture2D(R5_texture0, view.xy).r;
    vec3  normal    = normalize(normalMap.rgb * 2.0 - 1.0);
    float shininess = 4.0 + (normalMap.a * normalMap.a) * 250.0;
    float lightmap  = texture2D(R5_texture2, view.xy).r;

    // Transform the position from screen space to view space
    view.z = depth;
    vec4 clip = R5_inverseProjMatrix * vec4(view * 2.0 - 1.0, 1.0);
    view = clip.xyz / clip.w;

    // Light's attenuation is stored in the constant attenuation parameter
    vec3 light 	= gl_LightSource[0].position.xyz - view;
    float dist  = length(light);
    float atten = 1.0 - clamp(dist / gl_LightSource[0].constantAttenuation, 0.0, 1.0);

    // Light's power is stored as linear attenuation parameter
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
    //gl_FragData[0] = gl_LightSource[0].ambient * lightmap * atten + gl_LightSource[0].diffuse * (diffuseFactor * atten);

	// True AO shouldn't affect the diffuse channel. The line below is not really AO.
    gl_FragData[0] = gl_LightSource[0].ambient * (atten * lightmap) + gl_LightSource[0].diffuse * (diffuseFactor * atten * lightmap);

    // Specular component
    gl_FragData[1] = gl_LightSource[0].specular * (specularFactor * atten);
}
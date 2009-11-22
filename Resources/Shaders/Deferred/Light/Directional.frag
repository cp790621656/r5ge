uniform sampler2D   R5_texture0;    // Depth
uniform sampler2D   R5_texture1;    // View space normal and material shininess

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

    // Transform from screen space position to view space direction
    pos.z = depth;
    pos = (R5_inverseProjMatrix * vec4(pos * 2.0 - 1.0, 1.0)).xyz;

    vec4 diffuse  = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);

    // R5_FOR_EACH_LIGHT
    {
        vec3 incident = normalize(gl_LightSource[0].position.xyz);

        // Calculate contribution factors
        float diffuseFactor     = max( 0.0, dot(incident, normal) );
        float reflectiveFactor  = max( 0.0, dot(reflect(incident, normal), normalize(pos)) );
        float specularFactor    = pow( reflectiveFactor, shininess );

        // Diffuse and ambient component
        diffuse += gl_LightSource[0].ambient + gl_LightSource[0].diffuse * diffuseFactor;

        // Specular component
        specular += gl_LightSource[0].specular * specularFactor;
    }
    
    gl_FragData[0] = diffuse;
    gl_FragData[1] = specular;
}
#version 110

uniform samplerCube R5_texture0;
uniform sampler2D   R5_texture1;
uniform sampler2D   R5_texture2;

varying vec3 modelEyeDir, worldIncidentVector;
varying float fogFactor;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
    const float fresnelBias  = 0.1;  // Minimum reflectivity
    const float fresnelScale = 1.5;  // 1.00 - 20.0, the higher the value, the more reflection there is going to be
    const float fresnelPower = 2.0;  // 1.00 - 5.00, the higher the value, the less reflection there is going to be
    const float refractRatio = 0.91; // 0.85 - 1.00, the higher the value, the more focused the refraction is going to be

    // Normal information comes from the normal map texture
    vec3 normal0 = texture2D(R5_texture1, gl_TexCoord[0].st).xyz * 2.0 - 1.0;

    // Second texture's U and V are swapped, so swap X and Y
    vec3 normal1 = texture2D(R5_texture1, gl_TexCoord[1].st).yxz * 2.0 - 1.0;

    // Final normal is a simple combination of the two normals above
    vec3 normal  = normalize(normal0 + normal1);

    // World-space incident, reflection and refraction vectors
    vec3 incidentVector  = normalize(worldIncidentVector);
    vec3 reflectedVector = reflect(incidentVector, normal);
    //vec3 refractedVector = refract(incidentVector, normal, refractRatio);

    // Appoximation of the fresnel equation
    float fresnelFactor  = min( fresnelBias + fresnelScale * pow (1.0 + dot(incidentVector, normal), fresnelPower), 1.0 );

    // Model-space vectors for lighting calculations
    vec3 normalN         = normalize( vec3(gl_NormalMatrix * normal) );
    vec3 modelEyeDirN    = normalize(modelEyeDir);

    // Starting color is black, using the material's alpha
    vec4 color           = vec4(0.0, 0.0, 0.0, gl_FrontMaterial.diffuse.a);
    vec4 specular        = vec4(0.0, 0.0, 0.0, 0.0);
    
    // Directional light
    vec3 modelLightDirN = normalize(gl_LightSource[0].position.xyz);
    vec3 modelReflected = reflect(modelLightDirN, normalN);

    float diffuseFactor     = max( 0.0, dot(normalN, modelLightDirN) );
    float reflectiveFactor  = max( 0.0, dot(modelReflected, modelEyeDirN) );
    float specularFactor    = pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess );

    specular   += gl_FrontLightProduct[0].specular * specularFactor;
    color.rgb  += gl_FrontLightProduct[0].ambient.rgb;
    color.rgb  += gl_FrontLightProduct[0].diffuse.rgb * diffuseFactor;

    // Reflected color
    vec4 reflectedColor  = textureCube(R5_texture0, reflectedVector.xyz);

    // Height is sampled from the heightmap texture
    float height = texture2D(R5_texture2, gl_TexCoord[2].st).a * 35.0 - 3.0;
    float alphaFactor = 1.0 - clamp(height, 0.0, 2.0) * 0.5;

    // Mix all colors
    color       = mix(color, reflectedColor, fresnelFactor);
    color.a     = min(color.a, alphaFactor);
    color.rgb  += specular.rgb;
    color.rgb   = mix(color.rgb, gl_Fog.color.rgb, fogFactor);

    // Set the final color
    gl_FragColor = color;
}
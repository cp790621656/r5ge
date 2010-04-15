#version 110

uniform mat3 R5_inverseViewRotationMatrix;

uniform samplerCube R5_texture0;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
    const float fresnelBias     = 0.0;   // Minimum reflectivity
    const float fresnelScale    = 2.0;   // 1.00 - 20.0, the higher the more reflection there is
    const float fresnelPower    = 1.5;   // 1.00 - 5.00, the higher the less reflection there is
    const float refractRatioR   = 0.87;  // 0.85 - 1.00, the higher the more focused the refraction is
    const float refractRatioG   = 0.88;  // 0.85 - 1.00, the higher the more focused the refraction is
    const float refractRatioB   = 0.89;  // 0.85 - 1.00, the higher the more focused the refraction is

    vec3 normal = normalize(_normal);
    vec3 eyeDir = normalize(_eyeDir);

    // Reflect the directional vector, then transform the result into world space
    vec3 incident = R5_inverseViewRotationMatrix * reflect(eyeDir, normal);
    vec3 reflectedColor = textureCube(R5_texture0, incident).rgb;

    // Refracted vectors
    vec3 refractedR = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatioR);
    vec3 refractedG = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatioG);
    vec3 refractedB = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatioB);

    // Refracted color
    vec3 refractedColor = vec3( textureCube(R5_texture0, refractedR).r,
                                textureCube(R5_texture0, refractedG).g,
                                textureCube(R5_texture0, refractedB).b );

    // Appoximation of the fresnel equation
    float fresnelFactor = min( fresnelBias + fresnelScale * pow (1.0 + dot(eyeDir, normal), fresnelPower), 1.0 );

    // Mix reflected and refracted colors
    vec3 diffuse = mix(refractedColor, reflectedColor, fresnelFactor);

    // Mix reflected and refracted colors
    gl_FragData[0] = vec4(gl_FrontMaterial.diffuse.rgb * diffuse * gl_Color.rgb, gl_FrontMaterial.diffuse.a);
    gl_FragData[1] = vec4(gl_FrontMaterial.specular.rgb, gl_FrontMaterial.emission.a);
    gl_FragData[2] = vec4(normalize(normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}
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
    vec4 reflectedColor = textureCube(R5_texture0, incident);

    // Refracted vectors
    vec3 refractedR = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatioR);
    vec3 refractedG = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatioG);
    vec3 refractedB = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatioB);

    // Refracted color
    vec4 refractedColor;
    refractedColor.r = textureCube(R5_texture0, refractedR).r;
    refractedColor.g = textureCube(R5_texture0, refractedG).g;
    refractedColor.b = textureCube(R5_texture0, refractedB).b;
    refractedColor.a = 1.0;

    // Appoximation of the fresnel equation
    float fresnelFactor = min( fresnelBias + fresnelScale * pow (1.0 + dot(eyeDir, normal), fresnelPower), 1.0 );

    // Mix reflected and refracted colors
    gl_FragColor = mix(refractedColor, reflectedColor, fresnelFactor);
}
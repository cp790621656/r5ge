#version 110

uniform mat3 R5_inverseViewRotationMatrix;

uniform sampler2D   R5_texture0;
uniform samplerCube R5_texture1;

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _tangent;
varying vec3 _eyeDir;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
    const float fresnelBias  = 0.0;   // Minimum reflectivity
    const float fresnelScale = 2.0;   // 1.00 - 20.0, the higher the more reflection there is
    const float fresnelPower = 1.5;   // 1.00 - 5.00, the higher the less reflection there is
    const float refractRatio = 0.92;  // 0.85 - 1.00, the higher the more focused the refraction is

    // Normal map
    vec3 tangent = normalize(_tangent);
    vec3 normal  = normalize(_normal);
    mat3 TBN     = mat3(tangent, cross(normal, tangent), normal);
    normal       = TBN * normalize(texture2D(R5_texture0, _texCoord).xyz * 2.0 - 1.0);

    // View-space eye direction
    vec3 eyeDir  = normalize(_eyeDir);

    // Reflect the directional vector, then transform the result into world space
    vec3 incident = R5_inverseViewRotationMatrix * reflect(eyeDir, normal);
    vec3 reflectedColor  = textureCube(R5_texture1, incident).rgb;

    // Refracted color
    vec3 refractedVector = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatio);
    vec3 refractedColor  = textureCube(R5_texture1, refractedVector).rgb * gl_Color.rgb;

    // Appoximation of the fresnel equation
    float dotProduct = max(0.0, 1.0 + dot(eyeDir, normal));
    float fresnelFactor = min( fresnelBias + fresnelScale * pow(dotProduct, fresnelPower), 1.0 );

    // Mix reflected and refracted colors
    vec3 diffuse = mix(refractedColor, reflectedColor, fresnelFactor);

    // Encode the values
    gl_FragData[0] = vec4(gl_FrontMaterial.diffuse.rgb * diffuse, gl_FrontMaterial.diffuse.a);
    gl_FragData[1] = vec4(
		R5_MATERIAL_SPECULARITY,
		R5_MATERIAL_SPECULAR_HUE,
		R5_MATERIAL_GLOW, 1.0);
    gl_FragData[2] = vec4(normalize(normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}
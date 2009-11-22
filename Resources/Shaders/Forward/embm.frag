#version 110

uniform samplerCube R5_texture0; // Environment cube map
uniform sampler2D   R5_texture1; // Bump map

uniform mat3 R5_inverseViewRotationMatrix;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal, _tangent;
varying float _fogFactor;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
    vec3 tangent  = normalize(_tangent);
    vec3 normal   = normalize(_normal);
    mat3 TBN      = mat3(tangent, cross(normal, tangent), normal);
    normal        = normalize(TBN * (texture2D(R5_texture1, _texCoord).xyz * 2.0 - 1.0));
    vec3 eyeDir   = normalize(_eyeDir);

    // Reflect the directional vector, then transform the result into world space
    vec3 incident = R5_inverseViewRotationMatrix * reflect(eyeDir, normal);

    vec4 color = textureCube(R5_texture0, incident);
    color.rgb  = mix(color.rgb, gl_Fog.color.rgb, _fogFactor);

    gl_FragColor = color;
}
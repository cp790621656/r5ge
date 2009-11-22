#version 110

uniform sampler2D   R5_texture0; // Diffuse
uniform sampler2D   R5_texture1; // Normal map
uniform sampler2D   R5_texture2; // Specular(R), Reflection(G)
uniform samplerCube R5_texture3; // Environment map

uniform mat3 R5_inverseViewRotationMatrix;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal, _tangent;
varying float _fogFactor;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
    const float fresnelBias = 0.05;   // Minimum reflectivity
    const float fresnelPower = 1.0;   // 1.00 - 5.00, the higher the less reflection there is

    vec3 tangent  = normalize(_tangent);
    vec3 normal   = normalize(_normal);
    mat3 TBN      = mat3(tangent, cross(normal, tangent), normal);
    normal        = normalize(TBN * (texture2D(R5_texture1, _texCoord).xyz * 2.0 - 1.0));
    vec3 eyeDir   = normalize(_eyeDir);
    vec4 color    = vec4(0.0, 0.0, 0.0, gl_FrontMaterial.diffuse.a);
    vec4 specular = vec4(0.0);

    // Calculate light contribution
    // R5_FOR_EACH_LIGHT
    {
        vec3 lightDir = normalize(gl_TexCoord[0].xyz);

        float diffuseFactor    = max( 0.0, dot(normal, lightDir) );
        float reflectiveFactor = max( 0.0, dot(reflect(lightDir, normal), eyeDir) );
        float specularFactor   = pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess );

        color.rgb  += gl_FrontLightProduct[0].ambient.rgb * (gl_TexCoord[0].w);
        color.rgb  += gl_FrontLightProduct[0].diffuse.rgb * (gl_TexCoord[0].w * diffuseFactor);
        specular   += gl_FrontLightProduct[0].specular    * (gl_TexCoord[0].w * specularFactor);
    }
    
    // Reflect the directional vector, then transform the result into world space
    vec3 incident = R5_inverseViewRotationMatrix * reflect(eyeDir, normal);
    vec4 reflectedColor = textureCube(R5_texture3, incident);
    float fresnelFactor = min( fresnelBias + pow(1.0 + dot(eyeDir, normal), fresnelPower), 1.0 );

    // Combine the colors: diffuse texture, environment reflection, specular, and fog
    vec4 map   = texture2D(R5_texture2, _texCoord);
    color     *= texture2D(R5_texture0, _texCoord);
    color.rgb  = mix(color.rgb, reflectedColor.rgb, fresnelFactor * map.g);
    color.rgb += specular.rgb * map.r;
    color.rgb  = mix(color.rgb, gl_Fog.color.rgb, _fogFactor);
    color.a    = min(color.a, 1.0);

    gl_FragColor = color;
}
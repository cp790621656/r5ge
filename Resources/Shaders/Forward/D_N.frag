#version 110

uniform sampler2D   R5_texture0;
uniform sampler2D   R5_texture1;

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
    normal        = normalize(TBN * normalize(texture2D(R5_texture1, _texCoord).xyz * 2.0 - 1.0));
    vec3 eyeDir   = normalize(_eyeDir);
    vec4 color    = vec4(0.0, 0.0, 0.0, gl_FrontMaterial.diffuse.a);
    vec4 specular = vec4(0.0);

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

    color     *= texture2D(R5_texture0, _texCoord);
    color.rgb += specular.rgb;
    color.rgb  = mix(color.rgb, gl_Fog.color.rgb, _fogFactor);
    color.a    = min(color.a, 1.0);

    gl_FragColor = color;
}
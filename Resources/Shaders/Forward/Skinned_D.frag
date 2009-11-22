#version 110

uniform sampler2D   R5_texture0;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal;
varying float _fogFactor;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
    vec3 normal   = normalize(_normal);
    vec3 eyeDir   = normalize(_eyeDir);
    vec4 color    = vec4(0.0, 0.0, 0.0, gl_FrontMaterial.diffuse.a);
    vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);

    float diffuseFactor, reflectiveFactor, specularFactor;
    vec3 lightDir, reflected;

    // R5_FOR_EACH_LIGHT
    {
        if ( gl_LightSource[0].position.w == 0.0 )
        {
            // Directional light
            lightDir  = normalize(gl_LightSource[0].position.xyz);
            reflected = reflect(lightDir, normal);

            diffuseFactor     = max( 0.0, dot(normal, lightDir) );
            reflectiveFactor  = max( 0.0, dot(reflected, eyeDir) );
            specularFactor    = pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess );

            color.rgb  += gl_FrontLightProduct[0].ambient.rgb;
            color.rgb  += gl_FrontLightProduct[0].diffuse.rgb * diffuseFactor;
            specular   += gl_FrontLightProduct[0].specular * specularFactor;
        }
        else
        {
            // Point light
            lightDir      = gl_LightSource[0].position.xyz - _eyeDir;
            float dist    = length(lightDir);

            float atten = 1.0 - clamp(dist / gl_LightSource[0].constantAttenuation, 0.0, 1.0);
            atten = pow(atten, gl_LightSource[0].linearAttenuation);

            lightDir  = normalize(lightDir);
            reflected = reflect(lightDir, normal);

            diffuseFactor     = max( 0.0, dot(normal, lightDir) );
            reflectiveFactor  = max( 0.0, dot(reflected, eyeDir) );
            specularFactor    = pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess );

            color.rgb  += gl_FrontLightProduct[0].ambient.rgb * atten;
            color.rgb  += gl_FrontLightProduct[0].diffuse.rgb * (diffuseFactor  * atten);
            specular   += gl_FrontLightProduct[0].specular    * (specularFactor * atten);
        }
    }

    color.rgb   = mix(color.rgb, gl_FrontMaterial.emission.rgb, gl_FrontMaterial.emission.a);
    color      *= texture2D(R5_texture0, _texCoord);
    color.rgb  += specular.rgb;
    color.rgb   = mix(color.rgb, gl_Fog.color.rgb, _fogFactor);
    color.a     = min(color.a, 1.0);

    gl_FragColor = color;
}
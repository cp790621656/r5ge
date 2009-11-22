#version 110

uniform sampler2D   R5_texture0;
uniform sampler2D   R5_texture1;
uniform sampler2D   R5_texture2;
uniform samplerCube R5_texture3;

varying vec3 eyeDir, normal, reflectedEye;
varying float fogFactor;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
    vec3 normalN  = normalize(normal);
    vec3 eyeDirN  = normalize(eyeDir);
    vec4 color    = vec4(0.0, 0.0, 0.0, gl_FrontMaterial.diffuse.a);
    vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);

    float diffuseFactor, reflectiveFactor, specularFactor;
    vec3 lightDirN, reflectedN;

    for (int i = 0; i < R5_NUMBER_OF_LIGHTS; i++)
    {
        if ( gl_LightSource[i].position.w == 0.0 )
        {
            // Directional light
            lightDirN  = normalize(gl_LightSource[i].position.xyz);
            reflectedN = reflect(lightDirN, normalN);
    
        	diffuseFactor     = max( 0.0, dot(normalN, lightDirN) );
        	reflectiveFactor  = max( 0.0, dot(reflectedN, eyeDirN) );
            specularFactor    = pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess );
    
            specular   += gl_FrontLightProduct[i].specular * specularFactor;
            color.rgb  += gl_FrontLightProduct[i].ambient.rgb;
            color.rgb  += gl_FrontLightProduct[i].diffuse.rgb * diffuseFactor;
        }
        else
        {
            // Point light
            vec3 lightDir   = gl_LightSource[i].position.xyz - eyeDir;
            float distance  = length(lightDir);
            float atten     = 1.0 / (gl_LightSource[i].constantAttenuation  +
                                     gl_LightSource[i].linearAttenuation    * distance +
                                     gl_LightSource[i].quadraticAttenuation * distance * distance);
    
            lightDirN  = normalize(lightDir);
            reflectedN = reflect(lightDirN, normalN);
    
            diffuseFactor     = max( 0.0, dot(normalN, lightDirN) );
        	reflectiveFactor  = max( 0.0, dot(reflectedN, eyeDirN) );
            specularFactor    = pow( reflectiveFactor, 0.3 * gl_FrontMaterial.shininess );

            specular   += gl_FrontLightProduct[i].specular    * (specularFactor * atten);
            color.rgb  += gl_FrontLightProduct[i].ambient.rgb * atten;
            color.rgb  += gl_FrontLightProduct[i].diffuse.rgb * (diffuseFactor  * atten);
        }
    }

    vec4 color0 = texture2D(R5_texture0, gl_TexCoord[0].st);
    vec4 color1 = texture2D(R5_texture1, gl_TexCoord[0].st);
    vec4 color2 = texture2D(R5_texture2, gl_TexCoord[0].st);
    vec4 colorR = textureCube(R5_texture3, reflectedEye.xzy);

    color      *= mix(color2, color0, color1.a);
    color.rgb  += colorR.rgb * (1.0 - color1.a);
    color.rgb  += specular.rgb * (1.0 - color1.a);
    color.rgb   = mix(color.rgb, gl_Fog.color.rgb, fogFactor);
    color.a     = min(color.a, 1.0);

    gl_FragColor = color;
}
#version 110

uniform sampler2D   R5_texture0;
uniform sampler2D   R5_texture1;
uniform sampler2D   R5_texture2;
uniform sampler2D   R5_texture3;
uniform sampler2D   R5_texture4;
uniform sampler2D   R5_texture5;

uniform vec3 g_offset;
uniform vec3 g_scale;

varying float _fogFactor;
varying vec2 _texCoord, _mapCoord;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
    // Normal map contains the encoded XYZ normal along with the height value (alpha)
    vec4  map         = texture2D(R5_texture5, _mapCoord);
    float height      = (map.a + g_offset.z) * g_scale.z - pow(0.5, 3.0) * g_scale.z;
    vec3  normal      = normalize(map.xyz * 2.0 - 1.0);
    vec3  transNormal = normalize(gl_NormalMatrix * normal);

    const float heightCap = 20.0;

    // Starting color is black, using the material's alpha
    vec4 color = vec4(0.0, 0.0, 0.0, gl_FrontMaterial.diffuse.a);

    // Directional light
    float diffuseFactor = max( 0.0, dot(transNormal, normalize(gl_LightSource[0].position.xyz)) );
    color.rgb  += gl_FrontLightProduct[0].ambient.rgb;
    color.rgb  += gl_FrontLightProduct[0].diffuse.rgb * diffuseFactor;

    // Textures
    vec4 stone    = texture2D(R5_texture0, _texCoord);
    vec4 sand     = texture2D(R5_texture1, _texCoord);
    vec4 grass    = texture2D(R5_texture2, _texCoord);
    vec4 mountain = texture2D(R5_texture3, _texCoord);
    vec4 snow     = texture2D(R5_texture4, _texCoord);

    // Slope is based on the normal
    float nDot = pow(dot(normal, vec3(0.0, 0.0, 1.0)) + 0.2, 20.0);

    // Grass and mountain textures are mixed based on the slope
    float snowFactor = smoothstep(19.0 / 35.0 * heightCap, 27.0 / 35.0 * heightCap, height);
    float mountainFactor = max(1.0 - nDot, smoothstep(8.0 / 35.0 * heightCap,  19.0 / 35.0 * heightCap, height));

    snowFactor = min(snowFactor + snowFactor * nDot, 1.0);

    vec4 snowMountain  = mix(mountain, snow, snowFactor);
    vec4 grassMountain = mix(grass, snowMountain, mountainFactor);

    // Stone and sand
    //vec4 mixer = mix(stone, sand, smoothstep(-3.5, -1.0, height));

    // Previous and grass/mountain
    float heightFactor = smoothstep(0.5, 4.0, height);
    //mixer = mix(mixer, grassMountain, heightFactor);

    // Add a moving shadow to the terrain
    color       *= grassMountain;
    if (height < 0.0) color = vec4(0.0, 0.5, 1.0, 1.0);
    color.rgb    = mix(color.rgb, gl_Fog.color.rgb, _fogFactor);
    color.a      = min(color.a, 1.0);
    gl_FragColor = color;
}
#version 110

uniform sampler2D   R5_texture0; // Sand
uniform sampler2D   R5_texture1; // Field moss
uniform sampler2D   R5_texture2; // Mountain moss
uniform sampler2D   R5_texture3; // Rocks
uniform sampler2D   R5_texture4; // Snow
uniform sampler2D   R5_texture5; // Normal map (RGB) and height (A)

varying float _fogFactor;
varying vec2 _texCoord, _mapCoord;

//============================================================================================================================
// Fragment Shader
//============================================================================================================================

void main()
{
    // Normal map contains the encoded XYZ normal along with the height value (alpha)
    vec4  normalMap = texture2D(R5_texture5, _mapCoord);
    vec3  normal    = normalize(normalMap.xyz * 2.0 - 1.0);
    vec3  eyeNormal = normalize(gl_NormalMatrix * normal);
    float height 	= normalMap.a;

    // Directional light
    float diffuseFactor = max( 0.0, dot(eyeNormal, normalize(gl_LightSource[0].position.xyz)) );

	// Starting color should be the lit material color
    vec4 color = vec4(
		gl_FrontLightProduct[0].ambient.rgb +
		gl_FrontLightProduct[0].diffuse.rgb * diffuseFactor,
		gl_FrontMaterial.diffuse.a);

    // Textures
    vec4 sand  = texture2D(R5_texture0, _texCoord);
    vec4 grass = texture2D(R5_texture1, _texCoord);
    vec4 moss  = texture2D(R5_texture2, _texCoord);
    vec4 rock  = texture2D(R5_texture3, _texCoord);
    vec4 snow  = texture2D(R5_texture4, _texCoord);

    // Where various textures should appear
    float rockToSnow  = smoothstep(0.65,  1.0,  height);
    float grassToRock = smoothstep(0.35,  0.65, height);
    float grassToMoss = smoothstep(0.15,  0.3,  height);
    float sandToGrass = smoothstep(0.075, 0.15, height);

    // Slope is based on the normal -- the more bent the normal, the rockier this pixel should be
    float slope = pow(dot(normal, vec3(0.0, 0.0, 1.0)), 50.0);

    // Snow should appear only on really high or on flat surfaces
    rockToSnow = min(rockToSnow + rockToSnow * slope, 1.0);

	// Rocks appear where the slope is steep
    grassToRock = max(1.0 - slope, grassToRock);

	// Mixing snow with rock textures
    vec4 snowRock = mix(rock, snow, rockToSnow);

	// Mixing previous result with the moss texture
    vec4 grassRock = mix(moss, snowRock, grassToRock);

    // Mixing sand with grass textures
    vec4 mixer = mix(sand, grass, sandToGrass);

    // Finally mix the two sets of textures together
    mixer = mix(mixer, grassRock, grassToMoss);

    color       *= mixer;
    color.rgb    = mix(color.rgb, gl_Fog.color.rgb, _fogFactor);
    color.a      = min(color.a, 1.0);
    gl_FragColor = color;
}
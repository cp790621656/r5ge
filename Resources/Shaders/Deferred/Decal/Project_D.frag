uniform sampler2D   R5_texture0;	// View-space depth texture
uniform sampler2D   R5_texture1;	// View-space normal
uniform sampler2D   R5_texture2;	// View-space diffuse texture
uniform sampler2D   R5_texture3;	// View-space specular texture
uniform sampler2D   R5_texture4;	// Projected diffuse texture

uniform mat4 R5_inverseProjMatrix;	// Inverse projection matrix
uniform vec2 R5_pixelSize;          // 0-1 factor size of the pixel

uniform vec4  g_pos;				// Decal's position in view space (XYZ) and scale (W)
uniform vec3  g_forward;			// Decal's forward vector in view space
uniform vec3  g_right;				// Decal's right vector in view space
uniform vec3  g_up;					// Decal's up vector in view space

//============================================================================================================
// Gets the view space position at the specified texture coordinates
//============================================================================================================

vec3 GetViewPos (in vec2 screenTC)
{
	float depth = texture2D(R5_texture0, screenTC).r;
	vec4 pos = vec4(screenTC.x, screenTC.y, depth, 1.0);
    pos.xyz = pos.xyz * 2.0 - 1.0;
    pos = R5_inverseProjMatrix * pos;
    return pos.xyz / pos.w;
}

//============================================================================================================

void main()
{
	// This pixel's texture coordinates
	vec2 screenTC = gl_FragCoord.xy * R5_pixelSize;

	// This pixel's relative-to-center position
    vec3 pos = (GetViewPos(screenTC) - g_pos.xyz) / g_pos.w;

    // Determine the texture coordinates for the projected texture
    vec2 tc = vec2( dot((pos + g_right), 	g_right),
					dot((pos + g_up), 		g_up) );
	float maxDist = dot((pos + g_forward), 	g_forward);

	// By default alpha should only be affected by this pixel's distance to the center along the Z
	float alpha = abs(maxDist - 1.0);

	// Discard fragments that lie outside of the box
	if ( max( alpha, max(abs(tc.x - 1.0), abs(tc.y - 1.0)) ) > 1.0 ) discard;

	// Make alpha more focused in the center
	alpha = 1.0 - pow(alpha, 4.0);

    // Get the diffuse, specular, and projected diffuse textures
    vec4 originalNormal 	= texture2D(R5_texture1, screenTC);
	vec4 originalDiffuse 	= texture2D(R5_texture2, screenTC);
    vec4 originalSpecular 	= texture2D(R5_texture3, screenTC);
	vec4 projDiffuse 		= texture2D(R5_texture4, tc * 0.5) * gl_FrontMaterial.diffuse;

	// Mix the two diffuse textures using the combined alpha
	projDiffuse = mix(originalDiffuse, projDiffuse, alpha * projDiffuse.a);

	// Draw the decal
	gl_FragData[0] = vec4(projDiffuse.rgb, projDiffuse.a);
	gl_FragData[1] = originalSpecular;
	gl_FragData[2] = originalNormal;
}
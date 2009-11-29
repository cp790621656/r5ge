uniform sampler2D   R5_texture0;	// View-space diffuse texture
uniform sampler2D   R5_texture1;	// View-space specular texture
uniform sampler2D   R5_texture2;	// View-space normal
uniform sampler2D   R5_texture3;	// View-space depth texture
uniform sampler2D   R5_texture4;	// Projected diffuse texture
uniform sampler2D   R5_texture5;	// Projected normal texture

uniform mat4 R5_inverseProjMatrix;	// Inverse projection matrix
uniform vec2 R5_pixelSize;          // 0-1 factor size of the pixel

uniform vec3  g_pos;				// Decal's position in view space
uniform float g_scale;				// Decal's scale
uniform vec3  g_forward;			// Decal's forward vector in view space
uniform vec3  g_right;				// Decal's right vector in view space
uniform vec3  g_up;					// Decal's up vector in view space

// Gets the view space position at the specified texture coordinates
vec3 GetViewPos (in vec2 texCoord)
{
	float depth = texture2D(R5_texture3, texCoord).r;
	vec4 pos = vec4(texCoord.x, texCoord.y, depth, 1.0);
    pos.xyz = pos.xyz * 2.0 - 1.0;
    pos = R5_inverseProjMatrix * pos;
    return pos.xyz / pos.w;
}

void main()
{
	// This pixel's texture coordinates
	vec2 texCoord = gl_FragCoord.xy * R5_pixelSize;

	// This pixel's relative-to-center position
    vec3 pos = (GetViewPos(texCoord) - g_pos) / g_scale;

	// Alpha should fade out as the projected texture gets closer to the edges
	float alpha = 1.0 - pow(length(pos), 5.0);

	// If this pixel is not visible, discard it
	if (alpha < 0.0) discard;

    // Get the encoded view space normal
	vec3 viewNormal = normalize(texture2D(R5_texture2, texCoord).xyz * 2.0 - 1.0);

	// Determine if the pixel underneath should even be affected by the projector
	float dotVal = dot(viewNormal, -g_forward);

	// If it's at an angle of more than 90 degrees, discard it
	if (dotVal < 0.0) discard;

	// Flip the value so it can be brought to the power of 2 (sharpens contrast)
	// The reason I don't just do a 'sqrt' is because this performs faster.
	dotVal = 1.0 - dotVal;

	// Alpha should choose the smallest of the two contribution values
	alpha = min(alpha, 1.0 - dotVal * dotVal);

    // Determine the texture coordinates for the projected texture
    vec2 tc = vec2( dot((pos + g_right) * 0.5, g_right),
					dot((pos + g_up)    * 0.5, g_up) );

	// Get the projected diffuse texture
	vec4 projDiffuse = texture2D(R5_texture4, tc) * gl_FrontMaterial.diffuse;

   	// Decode the view space normal into a vector
	vec3 up, tangent;

	// Determine the up and tangent vectors
	if (dot(viewNormal, g_right) < 0.99)
	{
    	up = cross(viewNormal, g_right);
		tangent = cross(up, viewNormal);
	}
	else
	{
		tangent = cross(g_up, viewNormal);
		up = cross(viewNormal, tangent);
	}

	// Calculate the rotation matrix that will convert our projected normal into normal map's texture space
    mat3 TBN = mat3(tangent, up, viewNormal);

	// Retrieve and transform the projected normal
	vec3 normal = TBN * (texture2D(R5_texture5, tc).xyz * 2.0 - 1.0);

	// Mix the normals together using the calculated alpha
	normal = normalize( mix(viewNormal, normal, alpha) );

	// Draw the decal
	gl_FragData[0] = mix(texture2D(R5_texture0, texCoord), projDiffuse, alpha);
	gl_FragData[1] = texture2D(R5_texture1, texCoord);
	gl_FragData[2] = vec4(normal * 0.5 + 0.5, 1.0);
}
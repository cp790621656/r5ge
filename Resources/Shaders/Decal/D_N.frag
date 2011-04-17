uniform sampler2D   R5_texture0;	// View-space depth texture
uniform sampler2D   R5_texture1;	// View-space normal
uniform sampler2D   R5_texture2;	// View-space diffuse texture
uniform sampler2D   R5_texture3;	// View-space specular texture
uniform sampler2D   R5_texture4;	// Projected diffuse texture
uniform sampler2D   R5_texture5;	// Projected normal texture

uniform vec2 R5_pixelSize;          // 0-1 factor size of the pixel
uniform mat4 g_mat;					// Inverse world-view-projection matrix
uniform vec3 g_forward;				// Decal's forward vector in view space
uniform vec3 g_right;				// Decal's right vector in view space
uniform vec3 g_up;					// Decal's up vector in view space

vec3 GetWorldPos (in vec2 screenTC)
{
	float depth = texture2D(R5_texture0, screenTC).r;
	vec4 pos = vec4(screenTC.x, screenTC.y, depth, 1.0);
	pos.xyz = pos.xyz * 2.0 - 1.0;
	pos = g_mat * pos;
	return pos.xyz / pos.w;
}

void main()
{
	// This pixel's texture coordinates
	vec2 screenTC = gl_FragCoord.xy * R5_pixelSize;

	// This pixel's relative-to-center position
	vec3 pos = GetWorldPos(screenTC);
	float py = abs(pos.y);
	float alpha = max(abs(pos.x), max(abs(pos.z), py));
	if (alpha > 1.0) discard;
	vec2 tc = pos.xz * 0.5 + 0.5;

	// Determine if the pixel underneath should even be affected by the projector by considering its normal
	vec3 viewNormal;
	{
		// Get the encoded view space normal
		viewNormal = normalize(texture2D(R5_texture1, screenTC).xyz * 2.0 - 1.0);

		// If it's at an angle of more than 90 degrees, discard it
		float dotVal = dot(viewNormal, -g_forward);
		if (dotVal < 0.0) discard;

		// Make alpha more focused in the center
		alpha = 1.0 - pow(py, 4.0);

		// Flip the value so it can be brought to the power of 2 (sharpens contrast)
		// The reason I don't just do a 'sqrt' is because this performs faster.
		dotVal = 1.0 - dotVal;

		// Alpha should choose the smallest of the two contribution values
		alpha = min(alpha, 1.0 - dotVal * dotVal);
	}

	// Retrieve the screen diffuse and specular textures in addition to the projected diffuse texture
	vec4 originalDiffuse  	= texture2D(R5_texture2, screenTC);
	vec4 originalSpecular 	= texture2D(R5_texture3, screenTC);
	vec4 projDiffuse 		= texture2D(R5_texture4, tc) * gl_FrontMaterial.diffuse;

	// Transform the projected normal into normal map texture space
	vec3 normal;
	{
		vec3 tangent, up;
	
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
		normal = TBN * (texture2D(R5_texture5, tc).xyz * 2.0 - 1.0);
	
		// Mix the normals together using the calculated alpha
		normal = normalize( mix(viewNormal, normal, alpha) );
	}

	// Mix the two diffuse textures using the combined alpha
	projDiffuse = mix(originalDiffuse, projDiffuse, alpha * projDiffuse.a);

	// Draw the decal
	gl_FragData[0] = vec4(projDiffuse.rgb, projDiffuse.a);
	gl_FragData[1] = originalSpecular;
	gl_FragData[2] = vec4(normal * 0.5 + 0.5, 1.0);
}
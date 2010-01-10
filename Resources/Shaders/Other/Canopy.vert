uniform vec3 R5_origin;

void main()
{
	vec4 vertex = gl_Vertex;
	vec3 normal;

	{
	    // View-space offset is calculated based on texture coordinates,
		// enlarged by the size (texCoord's Z)
		vec3 offset = gl_MultiTexCoord0.xyz;
	    offset.xy = (offset.xy * 2.0 - 1.0) * offset.z;
		offset.z *= 0.25;

		// Calculate the view-transformed vertex
	    vertex = gl_ModelViewMatrix * vertex;

		// Apply the view-space offset
		vertex.xyz += offset;

        // Leaf's normal is relative to the origin of the canopy
		normal = normalize( gl_NormalMatrix * (gl_Vertex.xyz - R5_origin) );
	}

	// Texture coordinates come as-is
	gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;

    // The leaf's tint depends on the dot product of the camera's direction and the leaf's normal.
	// The idea behind this is to make leaves on the opposite side of the canopy darker, simulating depth.
	// Normal's Z is currently in -1 to 1 range, but we want it in 0.2 to 1.0 range.
	float tint = min(1.0, 0.7 + normal.z * 0.5);
	gl_FrontColor = vec4(tint, tint, tint, 1.0);

    // Final position
	gl_Position = gl_ProjectionMatrix * vertex;
}
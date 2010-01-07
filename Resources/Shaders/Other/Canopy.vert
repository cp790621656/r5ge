uniform vec3 R5_origin;

void main()
{
	// Calculate the view-transformed vertex
    vec4 vertex = gl_ModelViewMatrix * gl_Vertex;

	// Leaf's normal is relative to the origin of the canopy
    vec3 normal = normalize( gl_NormalMatrix * (gl_Vertex.xyz - R5_origin) );

	// The leaf's tint depends on the dot product of the camera's direction and the leaf's normal.
	// The idea behind this is to make leaves on the opposite side of the canopy darker, simulating depth.
	// Normal's Z is currently in -1 to 1 range, but we want it in 0.3 to 1.0 range.
	float tint = min(1.0, 0.7 + normal.z * 0.4);

	// Final color
	gl_FrontColor = vec4(tint, tint, tint, 1.0);

	// Texture coordinates are in the XY portion of texCoords
	vec3 offset = gl_MultiTexCoord0.xyz;
	gl_TexCoord[0].xy = offset.xy;

	// View-space offset is calculated based on texture coordinates, enlarged by the size (texCoord's Z)
    offset.xy = (offset.xy * 2.0 - 1.0) * offset.z;
	offset.z *= 0.25;

	// Apply the view-space offset
	vertex.xyz += offset;

	// Final position
	gl_Position = gl_ProjectionMatrix * vertex;
}
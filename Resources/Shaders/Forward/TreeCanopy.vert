uniform vec3 R5_time;
uniform vec3 R5_origin;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal, _tangent;
varying float _fogFactor;

void main()
{
	// Wind direction
	const vec3 wind = vec3(-0.783, -0.535, -0.07);

	// How much the billboards will spin as they move
	const float spinAmount = 1.5;

    // Start with the world space vertex
	vec4 vertex = gl_Vertex;

	// The higher the vertex, the more it is affected by the wind
	float windStrength = R5_time.y * max(0.0, vertex.z);

    // Calculate the normal and tangent
	vec3 offset	= gl_Vertex.xyz - R5_origin;
	_normal   	= normalize(offset);
	_tangent  	= normalize(vec3(offset.y, -offset.x, 0.0));

	// R5_IMPLEMENT_INSTANCING vertex _normal _tangent

	// Wind offset should take the vertex position into account
 	float vertexOffset = dot(vertex.xyz, wind) * 0.15;
 	float windOffset = sin(R5_time.x - vertexOffset);

	// Canopy billboards need to spin around ever-so-slightly
	float spin = vertexOffset + (spinAmount + sin(windStrength * 0.1)) * windOffset * 0.05;

	// Offset the vertex in world space
	vertex.xyz += wind * ((windOffset * 0.025 + 0.025) * windStrength);

	// Calculate the view-transformed vertex
	vertex = gl_ModelViewMatrix * vertex;

	// View-space offset is based on texture coordinates
	offset = gl_MultiTexCoord0.xyz;
	offset.xy = (offset.xy * 2.0 - 1.0) * offset.z;
	offset.z *= 0.25;

	// Rotate the particles slightly
	float s = sin(spin);
	float c = cos(spin);
	vertex.xyz += vec3(	offset.x * c - offset.y * s,
			  		   	offset.x * s + offset.y * c,
						offset.z );

	// Calculate the remaining values
	gl_Position = gl_ProjectionMatrix * vertex;
	_normal 	= gl_NormalMatrix * _normal;
    _tangent	= gl_NormalMatrix * _tangent;
	_texCoord	= gl_MultiTexCoord0.xy;
	_eyeDir 	= vertex.xyz;
	_fogFactor	= clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}

attribute vec3 R5_tangent;

uniform vec3 R5_time;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal, _tangent;
varying float _fogFactor;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
	// Wind direction
	const vec3 wind = vec3(-0.783, -0.535, -0.07);

	// Start with the world space vertex
	vec4 vertex = gl_Vertex;

	// The higher the vertex, the more it is affected by the wind
	float windStrength = R5_time.y * max(0.0, vertex.z);
	
	_normal  = gl_Normal;
	_tangent = R5_tangent;

	// R5_IMPLEMENT_INSTANCING vertex _normal _tangent
	
	// Wind offset should take the vertex position into account
 	float vertexOffset = dot(vertex.xyz, wind) * 0.15;
 	float windOffset = sin(R5_time.x - vertexOffset);

 	// Offset the vertex in world space
	vertex.xyz += wind * ((windOffset * 0.025 + 0.025) * windStrength);

	// Calculate the remaining values
	gl_Position = gl_ModelViewProjectionMatrix * vertex;
    _normal 	= gl_NormalMatrix * _normal;
    _tangent	= gl_NormalMatrix * _tangent;
    _texCoord   = gl_MultiTexCoord0.xy;
    _eyeDir     = (gl_ModelViewMatrix * gl_Vertex).xyz;
    _fogFactor  = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
// R5_INCLUDE Shaders/Deferred/D_N.frag

attribute vec3 R5_tangent;

uniform vec3 R5_time;
uniform float R5_worldScale;

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _tangent;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
	// Wind direction
	const vec3 wind = vec3(-0.783, 0.535, -0.07);

	// Start with the world space vertex
	vec4 vertex = gl_Vertex;

	// The higher the vertex, the more it is affected by the wind
	float windStrength = R5_time.y * max(0.0, vertex.z);
	
	_normal  = gl_Normal;
	_tangent = R5_tangent;

	// R5_IMPLEMENT_INSTANCING vertex _normal _tangent
	
	// Wind offset should take the vertex position into account
 	float vertexOffset = dot(vertex.xyz, wind / R5_worldScale) * 0.15;
 	float windOffset = sin(R5_time.x - vertexOffset);

 	// Offset the vertex in world space
	vertex.xyz += wind * ((windOffset * 0.025 + 0.025) * windStrength * R5_worldScale);

	// Calculate the remaining values
	gl_Position 	= gl_ModelViewProjectionMatrix * vertex;
	gl_FrontColor	= gl_Color;
    _normal 		= gl_NormalMatrix * _normal;
    _tangent		= gl_NormalMatrix * _tangent;
    _texCoord   	= gl_MultiTexCoord0.xy;
}
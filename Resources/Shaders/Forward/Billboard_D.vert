varying vec2 _texCoord;
varying vec3 _eyeDir, _normal;
varying float _fogFactor;

void main()
{
	vec4 vertex = gl_Vertex;

    // R5_IMPLEMENT_INSTANCING vertex
	// R5_IMPLEMENT_BILLBOARDING vertex _normal

	// Final position
	gl_Position = gl_ProjectionMatrix * vertex;
	_texCoord   = gl_MultiTexCoord0.xy;
    _eyeDir     = vertex.xyz;
    _fogFactor  = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
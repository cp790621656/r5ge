uniform vec3 g_offset;
uniform vec3 g_scale;

varying float _fogFactor;
varying vec2 _texCoord, _mapCoord;
varying vec3 _eyeDir, _tangent;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    _eyeDir     = (gl_ModelViewMatrix * gl_Vertex).xyz;
    _tangent	= gl_NormalMatrix * vec3(1.0, 0.0, 0.0);
    _texCoord  	= gl_Vertex.xy;
    _mapCoord  	= (gl_Vertex.xy - g_offset.xy) / g_scale.xy;
    _fogFactor 	= clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
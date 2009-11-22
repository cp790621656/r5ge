uniform vec3 g_offset;
uniform vec3 g_scale;

varying float _fogFactor;
varying vec2 _texCoord, _mapCoord;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
    gl_Position = ftransform();

    _texCoord  = gl_Vertex.xy * 0.1;
    _mapCoord  = gl_Vertex.xy / g_scale.xy - g_offset.xy;
    _fogFactor = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
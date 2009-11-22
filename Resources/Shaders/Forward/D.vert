varying vec2 _texCoord;
varying vec3 _eyeDir, _normal;
varying float _fogFactor;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
	gl_Position 	= gl_ModelViewProjectionMatrix * gl_Vertex;
    _texCoord   	= gl_MultiTexCoord0.xy;
    _eyeDir     	= (gl_ModelViewMatrix * gl_Vertex).xyz;
    _normal     	= gl_NormalMatrix * gl_Normal;
    _fogFactor  	= clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
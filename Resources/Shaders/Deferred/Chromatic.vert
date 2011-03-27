#version 110

varying vec3 _normal;
varying vec3 _eyeDir;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
    gl_Position 	= ftransform();
	gl_FrontColor 	= gl_Color;
    _normal     	= gl_NormalMatrix * gl_Normal;
    _eyeDir     	= (gl_ModelViewMatrix * gl_Vertex).xyz;
}
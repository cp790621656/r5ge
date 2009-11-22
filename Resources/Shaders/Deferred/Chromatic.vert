#version 110

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _eyeDir;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
    gl_Position = ftransform();

    _texCoord   = gl_MultiTexCoord0.xy;
    _normal     = gl_NormalMatrix * gl_Normal;
    _eyeDir     = (gl_ModelViewMatrix * gl_Vertex).xyz;
}
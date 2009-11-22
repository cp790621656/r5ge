#version 110

attribute vec3 R5_tangent;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal, _tangent;
varying float _fogFactor;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
    gl_Position    = ftransform();

    _texCoord   = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    _eyeDir     = (gl_ModelViewMatrix * gl_Vertex).xyz;
    _normal     = gl_NormalMatrix * gl_Normal;
    _tangent    = gl_NormalMatrix * R5_tangent;
}
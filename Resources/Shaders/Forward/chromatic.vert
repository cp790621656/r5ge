#version 110

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal;
varying float _fogFactor;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
    gl_Position = ftransform();
    _texCoord   = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    _eyeDir     = (gl_ModelViewMatrix * gl_Vertex).xyz;
    _normal     = gl_NormalMatrix * gl_Normal;
}
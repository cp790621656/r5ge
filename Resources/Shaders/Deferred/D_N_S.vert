attribute vec3 R5_tangent;

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _tangent;

void main()
{
    gl_Position = ftransform();
    _texCoord   = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    _normal     = gl_NormalMatrix * gl_Normal;
    _tangent    = gl_NormalMatrix * R5_tangent;
}
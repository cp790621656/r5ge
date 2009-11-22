varying vec2 _texCoord;
varying vec3 _normal;

void main()
{
    gl_Position = ftransform();
    _texCoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    _normal = gl_NormalMatrix * gl_Normal;
}
varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _eyeDir;

void main()
{
    gl_Position = ftransform();
    _eyeDir     = (gl_ModelViewMatrix * gl_Vertex).xyz;
    _texCoord   = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    _normal     = gl_NormalMatrix * gl_Normal;
}
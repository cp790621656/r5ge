varying vec3 _normal;

void main()
{
    gl_Position = ftransform();
    _normal = gl_NormalMatrix * gl_Normal;
}
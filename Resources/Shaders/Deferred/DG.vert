varying vec2 _texCoord;
varying vec3 _normal;
varying float _glow;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	_glow = clamp(gl_Vertex.z * 0.25, 0.0, 1.0);
	_texCoord = gl_MultiTexCoord0.xy;
	_normal = gl_NormalMatrix * gl_Normal;
}
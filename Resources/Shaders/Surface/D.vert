varying vec2 _texCoord;
varying vec3 _normal;

void main()
{
	_texCoord = gl_MultiTexCoord0.xy;
	_normal   = gl_NormalMatrix * gl_Normal;

	// R5_VERTEX_OUTPUT gl_Vertex gl_Color
}

varying vec3 _normal;

void main()
{
	vec4 vertex = gl_Vertex;
	vec3 normal = gl_Normal;

	// R5_IMPLEMENT_INSTANCING vertex normal

	_normal = gl_NormalMatrix * normal;

	// R5_VERTEX_OUTPUT vertex gl_Color
}

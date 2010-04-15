varying vec3 _normal;

void main()
{
	vec4 vertex = gl_Vertex;
	vec3 normal = gl_Normal;

	// R5_IMPLEMENT_INSTANCING vertex normal

	gl_Position 	= gl_ModelViewProjectionMatrix * vertex;
	gl_FrontColor 	= gl_Color;
	_normal 		= gl_NormalMatrix * normal;
}
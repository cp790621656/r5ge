// R5_INCLUDE Shaders/Deferred/D.frag

varying vec2 _texCoord;
varying vec3 _normal;

void main()
{
	vec4 vertex = gl_Vertex;
	vec3 normal = gl_Normal;

	// R5_IMPLEMENT_SKINNING vertex normal

	gl_Position 	= gl_ModelViewProjectionMatrix * vertex;
	_texCoord 		= gl_MultiTexCoord0.xy;
	_normal 		= gl_NormalMatrix * normal;
}
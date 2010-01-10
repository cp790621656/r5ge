varying vec2 _texCoord;
varying vec3 _normal;

void main()
{
    vec4 vertex = gl_Vertex;
	vec3 normal = gl_Normal;

    // R5_IMPLEMENT_SKINNING vertex normal
    // R5_IMPLEMENT_INSTANCING vertex normal

	gl_Position 	= gl_ModelViewProjectionMatrix * vertex;
    _texCoord 		= (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    _normal 		= gl_NormalMatrix * normal;
}
//============================================================================================================
// Vertex Shader used to transform geometry into a billboard texture
//============================================================================================================

attribute vec3 R5_tangent;

varying vec2 _texCoord;
varying vec3 _normal, _tangent;

void main()
{
    gl_FrontColor 	= gl_Color;
    gl_BackColor  	= gl_Color;
	gl_Position 	= gl_ModelViewProjectionMatrix * gl_Vertex;
    _normal     	= gl_NormalMatrix * gl_Normal;
    _tangent    	= gl_NormalMatrix * R5_tangent;
    _texCoord		= gl_MultiTexCoord0.xy;
}
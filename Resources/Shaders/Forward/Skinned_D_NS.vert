attribute vec3 R5_tangent;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal, _tangent;
varying float _fogFactor;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
	vec4 vertex   	= gl_Vertex;
	vec3 normal   	= gl_Normal;
	vec3 tangent  	= R5_tangent;

	// R5_IMPLEMENT_SKINNING vertex normal tangent
    // R5_IMPLEMENT_INSTANCING vertex normal tangent

	gl_Position 	= gl_ModelViewProjectionMatrix * vertex;
    _texCoord   	= gl_MultiTexCoord0.xy;
    _eyeDir     	= (gl_ModelViewMatrix * vertex).xyz;
    _normal     	= gl_NormalMatrix * normal;
    _tangent    	= gl_NormalMatrix * tangent;
    _fogFactor  	= clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
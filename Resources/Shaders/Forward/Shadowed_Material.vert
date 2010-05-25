uniform mat4 R5_shadowMatrix;

varying vec4 _inShadowPos;
varying vec2 _texCoord;
varying vec3 _eyeDir, _normal;
varying float _fogFactor;

void main()
{
	vec4 vertex = gl_Vertex;
	vec3 normal = gl_Normal;

    // R5_IMPLEMENT_INSTANCING vertex normal

	vec4 worldPos 	= gl_ModelViewMatrix * vertex;
	gl_Position 	= gl_ModelViewProjectionMatrix * vertex;
	_inShadowPos	= R5_shadowMatrix * worldPos;
    _texCoord   	= gl_MultiTexCoord0.xy;
    _eyeDir     	= worldPos.xyz;
    _normal     	= gl_NormalMatrix * normal;
    _fogFactor  	= clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
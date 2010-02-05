// R5_INCLUDE Shaders/Forward/D.frag

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal;
varying float _fogFactor;

void main()
{
	vec4 vertex = gl_Vertex;
	vec3 normal = gl_Normal;

    // R5_IMPLEMENT_SKINNING vertex normal

	gl_Position = gl_ModelViewProjectionMatrix * vertex;
    _texCoord   = gl_MultiTexCoord0.xy;
    _eyeDir     = (gl_ModelViewMatrix * vertex).xyz;
    _normal     = gl_NormalMatrix * normal;
    _fogFactor  = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
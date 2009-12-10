attribute vec3 R5_tangent;
attribute vec4 R5_boneWeight;
attribute vec4 R5_boneIndex;

uniform mat4 R5_boneTransforms[32];

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal, _tangent;
varying float _fogFactor;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
	mat4 transMat = R5_boneTransforms[int(R5_boneIndex.x)] * R5_boneWeight.x +
					R5_boneTransforms[int(R5_boneIndex.y)] * R5_boneWeight.y +
					R5_boneTransforms[int(R5_boneIndex.z)] * R5_boneWeight.z +
					R5_boneTransforms[int(R5_boneIndex.w)] * R5_boneWeight.w;

	mat3 rotMat   	= mat3(transMat[0].xyz, transMat[1].xyz, transMat[2].xyz);
	vec4 vertex   	= transMat * gl_Vertex;
	vec3 normal   	= rotMat   * gl_Normal;
	vec3 tangent  	= rotMat   * R5_tangent;
	
	//mat4 worldMat	= mat4(gl_MultiTexCoord2, gl_MultiTexCoord3, gl_MultiTexCoord4, gl_MultiTexCoord5);
	//mat3 worldRot	= mat3(gl_MultiTexCoord2.xyz, gl_MultiTexCoord3.xyz, gl_MultiTexCoord4.xyz);
	//vertex 			= worldMat * vertex;
	//normal 			= worldRot * normal;
	//tangent 		= worldRot * tangent;

	gl_Position 	= gl_ModelViewProjectionMatrix * vertex;
    _texCoord   	= gl_MultiTexCoord0.xy;
    _eyeDir     	= (gl_ModelViewMatrix * vertex).xyz;
    _normal     	= gl_NormalMatrix * normal;
    _tangent    	= gl_NormalMatrix * tangent;
    _fogFactor  	= clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
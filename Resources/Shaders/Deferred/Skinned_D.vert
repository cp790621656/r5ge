#version 120

attribute vec4 R5_boneWeight;
attribute vec4 R5_boneIndex;

uniform mat4 R5_boneTransforms[32];

varying vec2 _texCoord;
varying vec3 _normal;

void main()
{
    mat4 transMat = R5_boneTransforms[int(R5_boneIndex.x)] * R5_boneWeight.x +
					R5_boneTransforms[int(R5_boneIndex.y)] * R5_boneWeight.y +
					R5_boneTransforms[int(R5_boneIndex.z)] * R5_boneWeight.z +
					R5_boneTransforms[int(R5_boneIndex.w)] * R5_boneWeight.w;

	mat3 rotMat   	= mat3(transMat[0].xyz, transMat[1].xyz, transMat[2].xyz);
	vec4 vertex   	= transMat * gl_Vertex;
	vec3 normal   	= rotMat   * gl_Normal;

	gl_Position 	= gl_ModelViewProjectionMatrix * vertex;
    _texCoord 		= (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    _normal 		= gl_NormalMatrix * normal;
}
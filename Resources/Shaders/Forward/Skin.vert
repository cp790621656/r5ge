#version 120

attribute vec4 R5_boneWeight;
attribute vec4 R5_boneIndex;

uniform mat4 R5_boneTransforms[32];

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
	int   index;
	float weight;
	vec4  vertex  = vec4(0.0);

    weight   = R5_boneWeight.x;
	index    = int(R5_boneIndex.x);
	vertex  += (R5_boneTransforms[index] * gl_Vertex) * weight;
	
	weight   = R5_boneWeight.y;
	index    = int(R5_boneIndex.y);
	vertex  += (R5_boneTransforms[index] * gl_Vertex) * weight;

	weight   = R5_boneWeight.z;
	index    = int(R5_boneIndex.z);
	vertex  += (R5_boneTransforms[index] * gl_Vertex) * weight;

	weight   = R5_boneWeight.w;
	index    = int(R5_boneIndex.w);
	vertex  += (R5_boneTransforms[index] * gl_Vertex) * weight;

	gl_Position = gl_ModelViewProjectionMatrix * vertex;
}
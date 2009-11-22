uniform vec3 R5_eyePosition;

varying vec3 eyeDir, normal, reflectedEye;
varying float fogFactor;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
    gl_Position    = ftransform();
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    eyeDir        = vec3(gl_ModelViewMatrix * gl_Vertex);
    normal        = vec3(gl_NormalMatrix    * gl_Normal);
    reflectedEye  = reflect(gl_Vertex.xyz - R5_eyePosition, gl_Normal);

    fogFactor = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
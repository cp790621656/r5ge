uniform float R5_timeFactor;
uniform mat4  R5_worldTransformMatrix;
uniform vec3  R5_worldEyePosition;

varying vec3 modelEyeDir, worldIncidentVector;
varying float fogFactor;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
    gl_Position = ftransform();

    // World space vertex position
    vec3 worldVertex = (R5_worldTransformMatrix * gl_Vertex).xyz;
    worldIncidentVector = worldVertex - R5_worldEyePosition;
    modelEyeDir = (gl_ModelViewMatrix * gl_Vertex).xyz;

    // First texture's position is straightforward
    gl_TexCoord[0].st = worldVertex.xy * 0.1 + R5_timeFactor * 0.026;

    // Second texture's U and V are swapped. This also means that the fragment shader has to flip X and Y in the normal.
    gl_TexCoord[1].st = worldVertex.yx * 0.1 - R5_timeFactor * 0.024 + vec2(0.0, 0.5);

    // Fog factor is based on distance from the eye
    fogFactor = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);
}
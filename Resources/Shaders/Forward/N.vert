attribute vec3 R5_tangent;

varying vec2 _texCoord;
varying vec3 _eyeDir, _normal, _tangent;
varying float _fogFactor;

//============================================================================================================================
// Vertex Shader
//============================================================================================================================

void main()
{
    gl_Position = ftransform();
    _texCoord   = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
    _eyeDir     = (gl_ModelViewMatrix * gl_Vertex).xyz;
    _normal     = gl_NormalMatrix * gl_Normal;
    _tangent    = gl_NormalMatrix * R5_tangent;
    _fogFactor  = clamp((gl_Position.z - gl_Fog.start) * gl_Fog.scale, 0.0, 1.0);

    // R5_FOR_EACH_LIGHT
    {
        if ( gl_LightSource[0].position.w == 0.0 )
        {
            gl_TexCoord[0].xyz = normalize(gl_LightSource[0].position.xyz);
            gl_TexCoord[0].w = 1.0;
        }
        else
        {
            vec3 eyeToLight = gl_LightSource[0].position.xyz - _eyeDir;
            float dist = length(eyeToLight);
            gl_TexCoord[0].xyz = normalize(eyeToLight);
            float atten = 1.0 - clamp(dist / gl_LightSource[0].constantAttenuation, 0.0, 1.0);
            gl_TexCoord[0].w = pow(atten, gl_LightSource[0].linearAttenuation);
        }
    }
}
#pragma lighting off

#if Vertex

void main()
{
	R5_vertexPosition 	= R5_position;
	R5_vertexNormal 	= R5_normal;
	R5_vertexColor 		= R5_color;
}

#else if Fragment

void main()
{
	const float fresnelBias     = 0.0;   // Minimum reflectivity
    const float fresnelScale    = 2.0;   // 1.00 - 20.0, the higher the more reflection there is
    const float fresnelPower    = 1.5;   // 1.00 - 5.00, the higher the less reflection there is
    const float refractRatioR   = 0.87;  // 0.85 - 1.00, the higher the more focused the refraction is
    const float refractRatioG   = 0.88;  // 0.85 - 1.00, the higher the more focused the refraction is
    const float refractRatioB   = 0.89;  // 0.85 - 1.00, the higher the more focused the refraction is

    vec3 normal = normalize(R5_vertexNormal);
    vec3 eyeDir = normalize(R5_vertexEye);

    // Reflect the directional vector, then transform the result into world space
    vec3 incident = R5_inverseViewRotationMatrix * reflect(eyeDir, normal);
    vec4 reflectedColor = SampleCube(0, incident);

    // Refracted vectors
    vec3 refractedR = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatioR);
    vec3 refractedG = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatioG);
    vec3 refractedB = R5_inverseViewRotationMatrix * refract(eyeDir, normal, refractRatioB);

    // Refracted color
    vec4 refractedColor = vec4( SampleCube(0, refractedR).r,
                                SampleCube(0, refractedG).g,
                                SampleCube(0, refractedB).b, 1.0 );

    // Appoximation of the fresnel equation
    float fresnelFactor = min( fresnelBias + fresnelScale * pow (1.0 + dot(eyeDir, normal), fresnelPower), 1.0 );

    // Mix reflected and refracted colors
    R5_surfaceColor = R5_vertexColor * R5_materialColor * mix(refractedColor, reflectedColor, fresnelFactor);
}

#endif
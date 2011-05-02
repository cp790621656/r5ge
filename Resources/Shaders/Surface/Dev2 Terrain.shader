#if Vertex

void main()
{
	R5_vertexPosition 	= R5_position;
	R5_vertexTexCoord0	= (R5_position.xy * 0.025) + 0.5;
	R5_vertexTexCoord1 	= R5_position.xy * 0.5;
}

#else if DepthOnly

void main()
{
	R5_finalColor[0] = vec4(1.0);
}

#else if Fragment

void main()
{
	vec4 colorMap	= Sample2D(1, R5_vertexTexCoord0);
	vec4 tex0		= Sample2D(2, R5_vertexTexCoord1);
	vec4 tex2		= Sample2D(4, R5_vertexTexCoord1);

	tex0.rgb *= colorMap.g;
	tex2.rgb *= colorMap.b;

    R5_surfaceColor = mix(tex0, tex2, colorMap.r);

#if Lit or Deferred
	vec4 normalMap	= Sample2D(0, R5_vertexTexCoord0);
	vec4 tex1		= Sample2D(3, R5_vertexTexCoord1);
	vec4 tex3		= Sample2D(5, R5_vertexTexCoord1);

    vec3 normal = normalize(normalMap.xyz * 2.0 - 1.0);
	vec3 tangent = vec3(1.0, 0.0, 0.0);
	mat3 TBN = mat3(tangent, cross(normal, tangent), normal);

    normalMap = mix(tex1, tex3, colorMap.r);
    normal = mat3(R5_viewMatrix) * normalize(TBN * (normalMap.xyz * 2.0 - 1.0));

	R5_surfaceNormal = normal;
	R5_surfaceSpecularity = R5_materialSpecularity * normalMap.w;
#endif
}

#endif
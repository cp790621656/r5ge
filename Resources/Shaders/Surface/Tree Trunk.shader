#if Vertex

void main()
{
	// Wind direction
	const vec3 wind = vec3(-0.783, 0.535, -0.07);

	// The higher the vertex, the more it is affected by the wind
	float height 		= max(0.0, R5_position.z);
	float colorTint		= 0.3 + 0.7 * min(height, 3.0) * 0.33333;
	float windStrength 	= R5_time.y * height;

    // Wind offset should take the vertex position into account
 	float vertexOffset 	= dot(R5_position, wind / R5_modelScale) * 0.5;
 	float windOffset 	= sin(R5_time.x - vertexOffset);

    // Offset the vertex in world space
    R5_vertexPosition 	= R5_position + wind * ((windOffset * 0.025 + 0.025) * windStrength);
    R5_vertexColor		= vec4(R5_color.rgb * colorTint, R5_color.a);
    R5_vertexTexCoord0 	= R5_texCoord0;

#if Lit or Deferred
    R5_vertexNormal  	= R5_normal;
	R5_vertexTangent 	= R5_tangent;
#endif
}

#else if Fragment

void main()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor * Sample2D(0, R5_vertexTexCoord0);

#if Lit or Deferred
	vec4 tex1 = Sample2D(1, R5_vertexTexCoord0);
	R5_surfaceNormal = NormalMapToNormal(tex1);
	R5_surfaceSpecularity = R5_materialSpecularity * tex1.a;
#endif
}

#endif
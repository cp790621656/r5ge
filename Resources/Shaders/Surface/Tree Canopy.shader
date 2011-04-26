#if Vertex

uniform vec3 R5_origin;

void main()
{
	// Wind direction
	const vec3 wind = vec3(-0.783, 0.535, -0.07);

	// How much the billboards will spin as they move
	const float spinAmount = 1.5;

	// The higher the vertex, the more it is affected by the wind
	float windStrength 	= R5_time.y * max(0.0, R5_position.z);

    // Wind offset should take the vertex position into account
 	float vertexOffset 	= dot(R5_position, wind / R5_modelScale) * 0.5;
 	float windOffset 	= sin(R5_time.x - vertexOffset);

 	// The vertex is offset outward from the origin of the cloud
	vec3 offset	= R5_position - R5_origin;

#if Lit or Deferred
    R5_vertexNormal  	= normalize(offset);
	R5_vertexTangent 	= normalize(vec3(offset.y, -offset.x, 0.0));
#endif

	// Offset the rotation a bit based on the vertex position so all billboards look unique
	float spin = ((spinAmount + sin(windStrength * 0.1)) * windOffset) * 0.05 + dot(offset, offset);

	// Offset the vertex in world space
	vec4 vertex = vec4(R5_position + R5_modelScale * wind * ((windOffset * 0.025 + 0.025) * windStrength), 1.0);

	// Calculate the view-transformed vertex
	vertex = R5_modelMatrix * vertex;
	vertex = R5_viewMatrix * vertex;

	// View-space offset is based on texture coordinates
	offset = R5_texCoord0.xyz;
	offset.xy = (offset.xy * 2.0 - 1.0) * offset.z;

#if DepthOnly
	offset.z = -offset.z;
#else
	offset.z *= 0.25;
#endif
	offset *= R5_modelScale;

    // Canopy billboards need to spin around ever-so-slightly
	float s = sin(spin);
	float c = cos(spin);
	vertex.xyz += vec3(	offset.x * c - offset.y * s,
			  		   	offset.x * s + offset.y * c,
						offset.z );
    // Final values
    R5_vertexTexCoord0 	= R5_texCoord0.xy;
    R5_vertexColor 		= R5_color;
    R5_viewPosition 	= vertex;
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
void Vertex()
{
	R5_vertexPosition 	= R5_vertex;
	R5_vertexNormal 	= R5_normal;
	R5_vertexTangent	= R5_tangent;
	R5_vertexColor 		= R5_color;
	R5_vertexTexCoord0 	= R5_texCoord0;
}

void Fragment()
{
	vec3 tangent = normalize(R5_vertexTangent);
	vec3 normal = normalize(R5_vertexNormal);
	mat3 TBN = mat3(tangent, cross(normal.xyz, tangent), normal);

	vec4 tex1 = Sample2D(1, R5_vertexTexCoord0);
	normal = tex1.xyz * 2.0 - 1.0;
	normal = normalize(TBN * normal);

	R5_surfaceColor = R5_vertexColor * R5_materialColor * Sample2D(0, R5_vertexTexCoord0);
	R5_surfaceNormal = vec4(normal, R5_materialShininess);
}
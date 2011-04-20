void Vertex()
{
	R5_vertexPosition 	= R5_vertex;
	R5_vertexTexCoord0 	= R5_texCoord0;
	R5_vertexColor 		= R5_color;
	R5_vertexNormal 	= R5_normal;
	R5_vertexTangent	= R5_tangent;
}

void Fragment()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor * Sample2D(0, R5_vertexTexCoord0);
	vec4 tex1 = Sample2D(1, R5_vertexTexCoord0);
	R5_surfaceNormal = NormalMapToNormal(tex1);
	R5_surfaceSpecularity = tex1.a;
}
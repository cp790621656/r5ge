void Vertex()
{
	R5_vertexPosition = R5_vertex;
	R5_vertexNormal = R5_normal;
	R5_vertexColor = R5_color;
}

void Fragment()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor;
}
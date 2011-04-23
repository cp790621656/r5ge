#if Vertex

void main()
{
	R5_vertexPosition = R5_vertex;
	R5_vertexNormal = R5_normal;
	R5_vertexColor = R5_color;
}

#else if Fragment

void main()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor;
}

#endif
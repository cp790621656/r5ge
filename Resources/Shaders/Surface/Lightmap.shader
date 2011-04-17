void Vertex()
{
	R5_vertexPosition 	= R5_vertex;
	R5_vertexNormal 	= R5_normal;
	R5_vertexColor 		= R5_color;
	R5_vertexTexCoord0 	= R5_texCoord0;
}

void Fragment()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor;
	R5_surfaceProps = vec4(
		R5_materialSpecularity,
		R5_materialSpecularHue,
		R5_materialGlow,
		Sample2D(0, R5_vertexTexCoord0).a);
}
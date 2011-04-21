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
	// Surface color is used if writing to color
	R5_surfaceColor = R5_vertexColor * R5_materialColor * Sample2D(0, R5_vertexTexCoord0);

	// Surface alpha is used if not writing to color (only to depth)
	//R5_surfaceAlpha = R5_vertexColor.a * R5_materialColor.a * Sample2D(0, R5_vertexTexCoord0).a;

	vec4 tex1 = Sample2D(1, R5_vertexTexCoord0);
	R5_surfaceNormal = NormalMapToNormal(tex1);
	R5_surfaceSpecularity = tex1.a;
	
	// NOTE: The depth-only mode (and to some extent -- the unlit mode's simplification
	// is making this more difficult than it should.
}
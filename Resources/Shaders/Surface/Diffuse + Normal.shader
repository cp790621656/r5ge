#if Vertex

void main()
{
	R5_vertexPosition 	= R5_vertex;
	R5_vertexTexCoord0 	= R5_texCoord0;
	R5_vertexColor 		= R5_color;

#if Lit
	R5_vertexNormal 	= R5_normal;
	R5_vertexTangent	= R5_tangent;
#endif
}

#else if Fragment

void main()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor * Sample2D(0, R5_vertexTexCoord0);

#if Lit
	vec4 tex1 = Sample2D(1, R5_vertexTexCoord0);
	R5_surfaceNormal = NormalMapToNormal(tex1);
	R5_surfaceSpecularity = tex1.a;
#endif
}
#endif
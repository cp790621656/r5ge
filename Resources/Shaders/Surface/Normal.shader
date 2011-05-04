#if Vertex

void main()
{
	R5_vertexPosition 	= R5_position;
	R5_vertexTexCoord0 	= R5_texCoord0;
	R5_vertexColor 		= R5_color;

#if Lit or Deferred
	R5_vertexNormal 	= R5_normal;
	R5_vertexTangent	= R5_tangent;
#endif
}

#else if Fragment

void main()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor;

#if Lit or Deferred
	vec4 tex1 = Sample2D(0, R5_vertexTexCoord0);
	R5_surfaceNormal = NormalMapToNormal(tex1);
	R5_surfaceSpecularity = R5_materialSpecularity * tex1.a;
#endif
}
#endif
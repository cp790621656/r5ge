#if Vertex

void main()
{
	R5_vertexPosition 	= R5_position;
	R5_vertexColor 		= R5_color;

#if Lit or Deferred
	R5_vertexNormal 	= R5_normal;
	R5_vertexTexCoord0 	= R5_texCoord0;
#endif
}

#else if Fragment

void main()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor;
#if Lit or Deferred
	R5_surfaceOcclusion = Sample2D(0, R5_vertexTexCoord0).a;
#endif
}

#endif
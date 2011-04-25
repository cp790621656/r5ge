#if Vertex

void main()
{
	R5_vertexPosition = R5_position;
	R5_vertexColor = R5_color;

#if Lit or Deferred
	R5_vertexNormal = R5_normal;
#endif
}

#else if Fragment

void main()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor;
}

#endif
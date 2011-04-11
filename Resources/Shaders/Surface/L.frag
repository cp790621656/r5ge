void main()
{
	R5_surfaceColor = R5_vertexColor * R5_materialColor;
	R5_surfaceProps = vec4(R5_materialSpecularity, R5_materialSpecularHue, R5_materialGlow, Sample2D(0, R5_vertexTexCoord0).a);
}
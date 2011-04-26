#if Vertex

void main()
{
	R5_vertexPosition 	= R5_position;
    R5_vertexNormal    	= R5_normal;
    R5_vertexTangent   	= R5_tangent;
    R5_vertexTexCoord0	= R5_texCoord0;
}

#else if Fragment

void main()
{
	vec4 diffuseMap		= Sample2D(0, R5_vertexTexCoord0);
	vec4 normalMap 		= Sample2D(1, R5_vertexTexCoord0);
	vec3 normal 		= NormalMapToNormal(normalMap);
    R5_finalColor[0]	= vec4(normal * 0.5 + 0.5, min(1.0, diffuseMap.a));
}

#endif
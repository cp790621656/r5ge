#if Vertex

uniform vec3 g_offset;
uniform vec3 g_scale;

void main()
{
	R5_vertexPosition 	= R5_position;
	R5_vertexColor 		= R5_color;
	R5_vertexTexCoord0	= R5_position.xy;
	R5_vertexTexCoord1	= (R5_position.xy - g_offset.xy) / g_scale.xy;
}

#else if DepthOnly

void main()
{
	R5_finalColor[0] = vec4(1.0);
}

#else if Fragment

void main()
{
	vec3 sand  		= Sample2D(0, R5_vertexTexCoord0).rgb;
	vec3 grass 		= Sample2D(1, R5_vertexTexCoord0).rgb;
	vec3 moss  		= Sample2D(2, R5_vertexTexCoord0).rgb;
	vec3 rock  		= Sample2D(3, R5_vertexTexCoord0).rgb;
	vec3 snow  		= Sample2D(4, R5_vertexTexCoord0).rgb;
	vec4 normalMap 	= Sample2D(5, R5_vertexTexCoord1);

	// Where various textures should appear
	float rockToSnow  = smoothstep(0.65,  0.95, normalMap.a);
	float grassToRock = smoothstep(0.40,  0.65, normalMap.a);
	float grassToMoss = smoothstep(0.15,  0.40, normalMap.a);
	float sandToGrass = smoothstep(0.075, 0.15, normalMap.a);

	// Slope is based on the normal -- the more bent the normal, the rockier this pixel should be
	float slope = normalMap.z * normalMap.z;
	slope *= slope;
	
	// Snow should appear only on really high or on flat surfaces
	rockToSnow = min(rockToSnow + rockToSnow * slope, 1.0);
	
	// Rocks appear where the slope is steep
	grassToRock = max(1.0 - slope, grassToRock);
	
	// Mixing snow with rock textures
	vec3 snowRock = mix(rock, snow, rockToSnow);

	// Mixing previous result with the moss texture
	vec3 grassRock = mix(moss, snowRock, grassToRock);
	
	// Mixing sand with grass textures
	vec3 mixer = mix(sand, grass, sandToGrass);
	
	// Finally mix the two sets of textures together
	mixer = mix(mixer, grassRock, grassToMoss);

	// Final color
	R5_surfaceColor = R5_vertexColor * R5_materialColor * vec4(mixer, 1.0);
	R5_surfaceNormal = mat3(R5_viewMatrix) * normalize(normalMap.xyz * 2.0 - 1.0);
}

#endif

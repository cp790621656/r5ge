uniform sampler2D   R5_texture0;	// AO texture
uniform sampler2D   R5_texture1;	// Depth texture
uniform vec2        R5_pixelSize;
uniform vec4        R5_clipRange;
uniform vec2 		properties;		// X = focus range, Y = power

float GetDistance (in vec2 texCoord)
{
	float depth = texture2D(R5_texture1, texCoord).r;
    return R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w);
}

float GetFactor (in float origin, in vec2 texCoord)
{
    float dist = GetDistance(texCoord);
    dist = (origin - dist) / properties.x;
    return min(abs(dist), 1.0);
}

float GetContribution(in float originalAO, in float dist, in vec2 texCoord)
{
	float currentAO = texture2D(R5_texture0, texCoord).r;
	float factor = GetFactor(dist, texCoord);
    return mix(currentAO, originalAO, factor);
}

void main()
{
    vec2 tc = gl_TexCoord[0].xy;

    float dist = GetDistance(tc);

    float o1 = R5_pixelSize.y * 1.5;
    float o2 = R5_pixelSize.y * 3.5;
    float o3 = R5_pixelSize.y * 5.5;

    float ao = texture2D(R5_texture0, tc).r;

	ao = GetContribution(ao, dist, vec2(tc.x - o3, tc.y)) +
		 GetContribution(ao, dist, vec2(tc.x - o2, tc.y)) +
		 GetContribution(ao, dist, vec2(tc.x - o1, tc.y)) +
		 GetContribution(ao, dist, vec2(tc.x + o1, tc.y)) +
		 GetContribution(ao, dist, vec2(tc.x + o2, tc.y)) +
		 GetContribution(ao, dist, vec2(tc.x + o3, tc.y));

    gl_FragColor = vec4(ao / 6.0);
}
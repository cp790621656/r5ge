uniform sampler2D   R5_texture0;
uniform sampler2D   R5_texture1;
uniform vec2        R5_pixelSize;
uniform vec4        R5_clipRange;

float GetLinearDepth (in vec2 texCoord)
{
    return texture2D(R5_texture1, texCoord).r * R5_clipRange.w;
}

float GetFactor (in float depth, in vec2 texCoord)
{
    const float focusRange = 0.5;
    float sampled = GetLinearDepth(texCoord);
    return min(abs((depth - sampled) / focusRange), 1.0);
}

vec4 GetContribution(in vec4 original, in float depth, in vec2 texCoord)
{
    return mix(texture2D(R5_texture0, texCoord), original, GetFactor(depth, texCoord));
}

void main()
{
    vec2 tc = gl_TexCoord[0].xy;

    float depth = GetLinearDepth(tc);

    float o1 = R5_pixelSize.y * 0.5;
    float o2 = R5_pixelSize.y * 2.5;

    vec4 original = texture2D(R5_texture0, tc);

    gl_FragColor = (GetContribution(original, depth, vec2(tc.x, tc.y - o2)) +
                    GetContribution(original, depth, vec2(tc.x, tc.y - o1)) +
                    GetContribution(original, depth, vec2(tc.x, tc.y + o1)) +
                    GetContribution(original, depth, vec2(tc.x, tc.y + o2))) * 0.25;
}
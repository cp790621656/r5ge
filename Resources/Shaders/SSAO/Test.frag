#define NUM_SAMPLES 8.0

uniform sampler2D R5_texture0; // Depth
uniform sampler2D R5_texture1; // Normal
uniform sampler2D R5_texture2; // Random
uniform vec2 R5_pixelSize;     // 0-1 factor size of the pixel
uniform vec4 R5_clipRange;     // Near/far clipping range

//============================================================================================================

float GetDepth (in vec2 texCoord)
{
    float depth = texture2D(R5_texture0, texCoord).r;
    return (R5_clipRange.z / (R5_clipRange.y - depth * R5_clipRange.w) - R5_clipRange.x) / R5_clipRange.w;
    //return R5_clipRange.y * R5_clipRange.x / (depth * (R5_clipRange.y - R5_clipRange.x) - R5_clipRange.y);
}

//============================================================================================================

vec3 GetNormal (in vec2 texCoord)
{
    vec3 normal = texture2D(R5_texture1, texCoord).xyz;
    return normalize( normal * 2.0 - 1.0 );
}

//============================================================================================================

vec4 GetRandom (vec2 texCoord)
{
    //return 0.5+(fract(sin(dot(texCoord.xy, vec2(12.9898,78.233))) * 43758.5453))*0.5;
    return texture2D(R5_texture2, texCoord);
}

//============================================================================================================

void main()
{    
    //initialize occlusion sum and gi color:
    float sum = 0.0;

    //get depth at current pixel:
    float prof = GetDepth(gl_TexCoord[0].st);

    //scale sample number with depth:
    float samples = floor(NUM_SAMPLES / (0.5 + prof) + 0.5);

    // Eye space normal
    vec3 normal = GetNormal(gl_TexCoord[0].xy);

    // Random vector
    vec4 randVector = GetRandom(gl_TexCoord[0].xy);

    //calculate kernel steps:
    float incx2 = R5_pixelSize.x * 8.0;
    float incy2 = R5_pixelSize.y * 8.0;

    int hf = int(samples * 0.5);

    //do the actual calculations:
    for (int i=-hf; i < hf; i++)
    {
        for (int j=-hf; j < hf; j++)
        {
            if (i != 0 || j!= 0)
            {
                vec2 coords = vec2(i * incx2, j * incy2) / prof;

                float prof2g = GetDepth ( gl_TexCoord[0].st + coords * randVector.xy );
                vec3  norm2g = GetNormal( gl_TexCoord[0].st + coords * randVector.zw );

                vec3 dist2 = vec3(coords, prof - prof2g);

                float coherence2 = dot(-coords, vec2(norm2g.xy));

                if (coherence2 > 0.0)
                {
                    float pformfactor2 = 0.5 * (1.0 - dot(normal, norm2g)) /
                        (3.14159265 * pow(abs(length(dist2 * 2.0)), 2.0) + 0.5);

                    sum += clamp(pformfactor2 * 0.2, 0.0, 1.0);
                }
            }
        }
    }

    float occlusion = 1.0 - (sum / samples);
    gl_FragColor = vec4(occlusion, occlusion, occlusion, 1.0);
}
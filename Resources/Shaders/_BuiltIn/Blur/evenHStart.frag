uniform sampler2D   R5_texture0;
uniform vec2        R5_pixelSize;
uniform float       threshold;

void main()
{
    vec2 tc = gl_TexCoord[0].xy;

    float o1 = R5_pixelSize.x * 0.5;
    float o2 = R5_pixelSize.x * 2.5;

	gl_FragColor = max( (max(texture2D(R5_texture0, vec2(tc.x - o2, tc.y)), threshold) +
		 				 max(texture2D(R5_texture0, vec2(tc.x - o1, tc.y)), threshold) +
						 max(texture2D(R5_texture0, vec2(tc.x + o1, tc.y)), threshold) +
						 max(texture2D(R5_texture0, vec2(tc.x + o2, tc.y)), threshold)) * 0.25 - threshold, 0.0 );
}
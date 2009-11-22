uniform sampler2D   R5_texture0;
uniform vec2        R5_pixelSize;

void main()
{
    vec2 tc = gl_TexCoord[0].xy;

    float px1 = R5_pixelSize.x * 1.5;
    float px2 = R5_pixelSize.x * 2.5;

    vec4 center = texture2D(R5_texture0, tc);
    vec4 close  = texture2D(R5_texture0, vec2(tc.x - px1, tc.y)) +
                  texture2D(R5_texture0, vec2(tc.x + px1, tc.y));
    vec4 far    = texture2D(R5_texture0, vec2(tc.x - px2, tc.y)) +
                  texture2D(R5_texture0, vec2(tc.x + px2, tc.y));

    gl_FragColor = center * 0.509434 +
                   close  * 0.169811 +
                   far    * 0.075472;
}
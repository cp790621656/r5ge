uniform sampler2D   R5_texture0;
uniform vec2        R5_pixelSize;

void main()
{
    vec2 tc = gl_TexCoord[0].xy;

    float o1 = R5_pixelSize.x * 0.5;
    float o2 = R5_pixelSize.x * 2.5;

    gl_FragColor = (texture2D(R5_texture0, vec2(tc.x - o2, tc.y)) +
                    texture2D(R5_texture0, vec2(tc.x - o1, tc.y)) +
                    texture2D(R5_texture0, vec2(tc.x + o1, tc.y)) +
                    texture2D(R5_texture0, vec2(tc.x + o2, tc.y))) * 0.25;
}
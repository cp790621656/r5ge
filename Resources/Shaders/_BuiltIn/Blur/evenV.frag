uniform sampler2D   R5_texture0;
uniform vec2        R5_pixelSize;

void main()
{
    vec2 tc = gl_TexCoord[0].xy;

    float o1 = R5_pixelSize.y * 0.5;
    float o2 = R5_pixelSize.y * 2.5;

    gl_FragColor = (texture2D(R5_texture0, vec2(tc.x, tc.y - o2)) +
                    texture2D(R5_texture0, vec2(tc.x, tc.y - o1)) +
                    texture2D(R5_texture0, vec2(tc.x, tc.y + o1)) +
                    texture2D(R5_texture0, vec2(tc.x, tc.y + o2))) * 0.25;
}
uniform sampler2D   R5_texture0;
uniform vec4        R5_clipRange;

void main()
{
    float depth = texture2D(R5_texture0, gl_TexCoord[0].xy).r;
    float val = (R5_clipRange.x - R5_clipRange.z / (depth * R5_clipRange.w - R5_clipRange.y)) / R5_clipRange.w;
    gl_FragColor = vec4(val, val, val, 1.0);
}
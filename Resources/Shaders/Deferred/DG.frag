uniform sampler2D   R5_texture0;

varying vec2 _texCoord;
varying vec3 _normal;

void main()
{
     vec4 diffuseMap = texture2D(R5_texture0, _texCoord);

    gl_FragData[0] = vec4(gl_FrontMaterial.diffuse.rgb * diffuseMap.rgb, gl_FrontMaterial.diffuse.a);
    gl_FragData[1] = vec4(gl_FrontMaterial.specular.rgb, gl_FrontMaterial.emission.a + diffuseMap.a);
    gl_FragData[2] = vec4(normalize(_normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}
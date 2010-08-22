uniform sampler2D   R5_texture0; // Diffuse map
uniform sampler2D   R5_texture1; // Normal map
uniform sampler2D   R5_texture2; // Specular map

varying vec2 _texCoord;
varying vec3 _normal;
varying vec3 _tangent;

void main()
{
    // Normal map
    vec3 tangent = normalize(_tangent);
    vec3 normal  = normalize(_normal);
    mat3 TBN     = mat3(tangent, cross(normal, tangent), normal);
    normal       = TBN * normalize(texture2D(R5_texture1, _texCoord).xyz * 2.0 - 1.0);

    // Specular map
    vec4 texmap = vec4(texture2D(R5_texture2, _texCoord));

    // Encode the values
    gl_FragData[0] = gl_FrontMaterial.diffuse * texture2D(R5_texture0, _texCoord) * gl_Color;
	gl_FragData[1] = vec4(
		R5_MATERIAL_SPECULARITY * texmap.r,
		R5_MATERIAL_SPECULAR_HUE,
		R5_MATERIAL_GLOW, 1.0);
    gl_FragData[2] = vec4(normalize(normal) * 0.5 + 0.5, gl_FrontMaterial.specular.a);
}
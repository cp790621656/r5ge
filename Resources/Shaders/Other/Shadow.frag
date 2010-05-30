uniform sampler2D		R5_texture0;
uniform sampler2DShadow R5_texture1;
uniform mat4			shadowMatrix;
uniform vec2			lightDepthPixelSize;

void main()
{
	vec2 texCoord = gl_TexCoord[0].xy;
	float depth = texture2D(R5_texture0, texCoord).r;
	vec3 pos = vec3(texCoord.x, texCoord.y, depth);

	// Transform the screen coordinate to light space
	vec4 pos4 = vec4(pos, 1.0);
	pos4 = shadowMatrix * pos4;
	pos = pos4.xyz / pos4.w;

	// 30 degree rotated kernel (0.866, 0.5) multiplied by 2
	float offsetX = 1.732 * lightDepthPixelSize.x;
	float offsetY = lightDepthPixelSize.y;
	float shadowFactor = shadow2D(R5_texture1, pos).r;

	shadowFactor += shadow2D(R5_texture1, pos + vec3(-offsetX,  offsetY, 0.0)).r;
	shadowFactor += shadow2D(R5_texture1, pos + vec3(-offsetX, -offsetY, 0.0)).r;
	shadowFactor += shadow2D(R5_texture1, pos + vec3( offsetX, -offsetY, 0.0)).r;
	shadowFactor += shadow2D(R5_texture1, pos + vec3( offsetX,  offsetY, 0.0)).r;
	shadowFactor *= 0.2;

	gl_FragColor = vec4(shadowFactor);
}
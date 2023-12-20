#version 460 core
in vec2 texCoord;
in vec3 fragNormal;
out vec4 fragColor;

struct Material
{
	sampler2D texture_diffuse1;
};

uniform Material material;
uniform bool turnGreen;

void main()
{
	fragColor = texture(material.texture_diffuse1, texCoord);
	float brightness = ((dot(fragNormal, vec3(0, 1, 0))+1) * 0.5);
	if (turnGreen == true)
		fragColor = vec4(0.0, 1.0, 0.0, 1.0) * brightness;
}
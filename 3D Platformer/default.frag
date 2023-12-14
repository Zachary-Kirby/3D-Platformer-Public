#version 460 core
in vec2 texCoord;
out vec4 fragColor;

struct Material
{
	sampler2D texture_diffuse1;
};

uniform Material material;

void main()
{
	fragColor = texture(material.texture_diffuse1, texCoord);
}
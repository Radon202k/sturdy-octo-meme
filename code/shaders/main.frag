#version 450 core

in vec2 uv;
in vec4 color;

layout (binding=0)
uniform sampler2D s_texture;

layout (location=0)
out vec4 o_color;

void main()
{
	vec4 texelColor = texture(s_texture, uv);
	o_color = color * texelColor;
} 
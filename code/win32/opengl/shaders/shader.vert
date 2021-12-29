#version 450 core

layout (location=0) in vec2 a_pos;
layout (location=1) in vec2 a_uv;
layout (location=2) in vec3 a_color;

layout (location=0)
uniform mat2 u_matrix;

out gl_PerVertex { vec4 gl_Position; };
out vec2 uv;
out vec4 color;

void main()
{
	vec2 pos = u_matrix * a_pos;
	gl_Position = vec4(pos, 0, 1);
	uv = a_uv;
	color = vec4(a_color, 1);
}
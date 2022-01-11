#version 450 core

layout (location=0) in vec3 a_pos;
layout (location=1) in vec3 a_color;

layout (location=0)
uniform mat4 projview;

out gl_PerVertex { vec4 gl_Position; };
out vec4 color;

void main()
{
	vec4 pos = projview * vec4(a_pos, 1);
	gl_Position = pos;
	color = vec4(a_color, 1);
}
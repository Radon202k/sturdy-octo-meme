#version 450 core

layout (location=0) in vec4 inPos;

layout (location=0)
uniform mat4 projView;

out gl_PerVertex { vec4 gl_Position; };
out vec2 uv;

void main()
{
	float blockType = inPos.w;
	
	vec3 pos = projView * vec4(inPos.xyz, 1);
	
	int cube = gl_VertexID % 24;
	int face = gl_VertexID / 4;
	int vertex = gl_VertexID % 4;

	float u = (vertex % 2) * 1.0f;
	float v = ((vertex % 2) + 1) * 1.0f;

	float dX = ;

	uv = vec2(u, v);

	gl_Position = pos;
}
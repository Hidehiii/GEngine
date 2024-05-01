
#name PBR

#type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec4 i_normal;

layout(std140, binding = 0) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
layout(std140, binding = 1) uniform TIME
{
	float GE_TIME;
};
layout(std140, binding = 2) uniform MODEL
{
	mat4 GE_MATRIX_M;
};

struct VertexOutput
{
	vec4 color;
	vec4 normal;
};
layout (set = 1, location = 0) out VertexOutput OUT;
void main()
{
	OUT.color = i_color;
	OUT.normal = i_normal;
	gl_Position = GE_MATRIX_VP * GE_MATRIX_M * i_position;
}

#type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
layout(std140, binding = 0) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
layout(std140, binding = 2) uniform MODEL
{
	mat4 GE_MATRIX_M;
};
struct VertexOutput
{
	vec4 color;
	vec4 normal;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	vec4 c = normalize(GE_MATRIX_M * IN.normal);
	o_color = vec4(c.x, c.y, c.z, 1.0);
}
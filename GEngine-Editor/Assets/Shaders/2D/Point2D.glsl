
#name Point2D

#properties

#type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
layout(std140, binding = 1) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
struct VertexOutput
{
	vec4 color;
};
layout (location = 0) out VertexOutput OUT;
void main()
{
	OUT.color = i_color;
	gl_Position = GE_MATRIX_VP * i_position;
}

#type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
layout(std140, binding = 2) uniform TIME
{
	float GE_TIME;
};
struct VertexOutput
{
	vec4 color;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	o_color = IN.color;
}
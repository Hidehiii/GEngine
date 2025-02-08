
#Name Point2D

#Type vertex
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
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

#Type fragment
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 o_color;
struct VertexOutput
{
	vec4 color;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	o_color = IN.color;
}

#Name Present

#Properties

GE_PRESENT_FRAME_BUFFER: sampler2D

#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
struct VertexOutput
{
	vec2 uv;
};
layout (location = 0) out VertexOutput OUT;
void main()
{
	OUT.uv = i_uv;
	gl_Position =  i_position;
}

#Type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
struct VertexOutput
{
	vec2 uv;
	
};
layout (location = 0) in VertexOutput IN;
layout (binding = 10) uniform sampler2D GE_PRESENT_FRAME_BUFFER;
void main()
{
	o_color = texture(GE_PRESENT_FRAME_BUFFER, IN.uv);
}
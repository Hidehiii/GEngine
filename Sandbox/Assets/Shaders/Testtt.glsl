
#Name Testtt

#Properties

float prop1;
sampler2D tex1;


#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in int i_texIndex;
layout(std140, binding = 0) uniform MATERIAL
{
	float prop1;
};
layout(std140, binding = 1) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
struct VertexOutput
{
	vec4 position;
	vec2 uv;
	
};
layout (location = 0) out VertexOutput OUT;
layout (location = 2) flat out int v_TexIndex;
void main()
{
	OUT.uv = i_uv;
	
	v_TexIndex = i_texIndex;
	gl_Position =  GE_MATRIX_VP * i_position;
	gl_Position.y *= prop1;
	OUT.position = gl_Position;
}

#Type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
layout(std140, binding = 2) uniform TIME
{
	vec4 GE_TIME;
};
struct VertexOutput
{
	vec4 position;
	vec2 uv;
	
};
layout (location = 0) in VertexOutput IN;
layout (location = 2) in flat int v_TexIndex;
layout (binding = 10) uniform sampler2D tex1;
void main()
{
	o_color = vec4(IN.uv, 0.0f, 1.0f);
	o_color = texture(tex1, IN.uv);
}
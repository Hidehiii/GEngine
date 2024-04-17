
#name Quad2D

#type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec2 i_tiling;
layout(location = 3) in vec4 i_color;
layout(location = 4) in int i_texIndex;
layout(std140, binding = 0) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
struct VertexOutput
{
	vec2 uv;
	vec4 color;
	vec2 tiling;
};
layout (location = 0) out VertexOutput OUT;
layout (location = 3) flat out int v_TexIndex;
void main()
{
	OUT.uv = i_uv;
	OUT.color = i_color;
	OUT.tiling = i_tiling;
	v_TexIndex = i_texIndex;
	gl_Position = GE_MATRIX_VP * i_position;
}

#type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
layout(std140, binding = 1) uniform TIME
{
	float GE_TIME;
};
struct VertexOutput
{
	vec2 uv;
	vec4 color;
	vec2 tiling;
};
layout (location = 0) in VertexOutput IN;
layout (location = 3) in flat int v_TexIndex;
layout (binding = 0) uniform sampler2D _Textures[32];
void main()
{
	o_color = texture(_Textures[v_TexIndex], IN.uv * IN.tiling) * IN.color;
}
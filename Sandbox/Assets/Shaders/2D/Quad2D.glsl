
#Name Quad2D

#Properties

Tex_0: sampler2D
Tex_1: sampler2D
Tex_2: sampler2D
Tex_3: sampler2D
Tex_4: sampler2D
Tex_5: sampler2D
Tex_6: sampler2D
Tex_7: sampler2D

#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec2 i_tiling;
layout(location = 3) in vec4 i_color;
layout(location = 4) in int i_texIndex;
layout(std140, binding = 1) uniform CAMERA
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

#Type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
layout(std140, binding = 2) uniform TIME
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
layout (binding = 10) uniform sampler2D Tex_0;
layout (binding = 11) uniform sampler2D Tex_1;
layout (binding = 12) uniform sampler2D Tex_2;
layout (binding = 13) uniform sampler2D Tex_3;
layout (binding = 14) uniform sampler2D Tex_4;
layout (binding = 15) uniform sampler2D Tex_5;
layout (binding = 16) uniform sampler2D Tex_6;
layout (binding = 17) uniform sampler2D Tex_7;
void main()
{
	o_color = vec4(1.0f); 
	switch(v_TexIndex)
	{
		case 0: o_color = texture(Tex_0, IN.uv * IN.tiling) * IN.color; break;
		case 1: o_color = texture(Tex_1, IN.uv * IN.tiling) * IN.color; break;
		case 2: o_color = texture(Tex_2, IN.uv * IN.tiling) * IN.color; break;
		case 3: o_color = texture(Tex_3, IN.uv * IN.tiling) * IN.color; break;
		case 4: o_color = texture(Tex_4, IN.uv * IN.tiling) * IN.color; break;
		case 5: o_color = texture(Tex_5, IN.uv * IN.tiling) * IN.color; break;
		case 6: o_color = texture(Tex_6, IN.uv * IN.tiling) * IN.color; break;
		case 7: o_color = texture(Tex_7, IN.uv * IN.tiling) * IN.color; break;
	}
}
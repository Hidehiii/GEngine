#type vertex
#version 450 core

layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec2 i_tiling;
layout(location = 3) in vec4 i_color;
layout(location = 4) in float i_texIndex;

uniform mat4 GE_MATRIX_VP;
uniform float _Time;

out vec2 v_UV;
out vec4 v_Color;
out vec2 v_Tiling;
out float v_TexIndex;

void main()
{
	v_UV = i_uv;
	v_Color = i_color;
	v_Tiling = i_tiling;
	v_TexIndex = i_texIndex;
	//gl_Position = GE_MATRIX_VP * GE_MATRIX_M * i_position;
	gl_Position = GE_MATRIX_VP * i_position;
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_color;

uniform sampler2D _Textures[32];
uniform float _Time;


in vec2 v_UV;
in vec4 v_Color;
in vec2 v_Tiling;
in float v_TexIndex;
			

void main()
{
	o_color = texture(_Textures[int(v_TexIndex)], v_UV * v_Tiling) * v_Color;
}
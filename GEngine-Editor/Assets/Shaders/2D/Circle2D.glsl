
#Name Circle2D

#Properties

#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec2  i_uv;
layout(location = 3) in float i_radius;
layout(location = 4) in float  i_thickness;
layout(location = 5) in int  i_texIndex;
layout(location = 6) in float  i_fade;

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
	vec2 uv;
	float radius;
	float thickness;
	float fade;
};
layout (location = 0) out VertexOutput OUT;
layout (location = 5) flat out int v_TexIndex;
void main()
{
	OUT.color = i_color;
	OUT.uv = i_uv;
	OUT.radius = i_radius;
	OUT.thickness = i_thickness;
	OUT.fade = i_fade;
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
	vec4 color;
	vec2 uv;
	float radius;
	float thickness;
	float fade;
};
layout (location = 0) in VertexOutput IN;
layout (location = 5) in flat int v_TexIndex;
layout (binding = 0) uniform sampler2D _Textures[32];
void main()
{
	o_color = texture(_Textures[v_TexIndex], IN.uv) * IN.color;
	float dist = length(IN.uv - vec2(0.5f, 0.5f));
	if(dist > IN.radius || dist < IN.radius * (1 - IN.thickness))
	{
		discard;
	}
	if(IN.fade > 0)
	{
		if(IN.thickness >= 1)
		{
			o_color.a *= smoothstep(1.0f, 0.0f, (dist - IN.radius + IN.fade) / (IN.radius - IN.fade));
		}
	}
}
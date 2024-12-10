
#Name AlphaTestShader
#Blend Alpha SrcAlpha OneMinusSrcAlpha
#DepthWrite Off
#DepthTest On
#Properties
color p0;
vector p1;
float p2;


#Type vertex
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec4 i_normal;
layout(location = 3) in vec4 i_Tangent;
layout(location = 4) in vec2 i_UV0;
layout(location = 5) in vec2 i_UV1;
layout(location = 6) in vec2 i_UV2;
layout(location = 7) in vec2 i_UV3;
struct VertexOutput
{
	vec4 color;
	vec4 normal;
};
layout (location = 0) out VertexOutput OUT;
void main()
{
	OUT.color = i_color;
	OUT.normal = normalize(GE_MATRIX_M * i_normal);
	gl_Position = GE_MATRIX_VP * GE_MATRIX_M * i_position;
}

#Type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
layout(std140, binding = 0) uniform MATERIAL
{
	vec4 p0;
	vec4 p1;
	float p2;
};
struct VertexOutput
{
	vec4 color;
	vec4 normal;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	vec4 c = IN.normal;
	c.xyz = p0.xyz;
	o_color = vec4(c.x, c.y, c.z, p2);
}
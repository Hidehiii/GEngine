#Name ExampleShader
#Blend Add SrcAlpha OneMinusSrcAlpha
#DepthWrite On
#DepthTest LEqual
#Cull Back
#Properties

color prop1;
float prop2;
int prop3;
vector prop4;

#EndProperties

#Type vertex
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec4 i_normal;
struct VertexOutput
{
	vec4 color;
	vec4 normal;
};
layout (set = 1, location = 0) out VertexOutput OUT;
void main()
{
	OUT.color = i_color;
	OUT.normal = normalize(GE_MATRIX_M * i_normal);
	gl_Position = GE_MATRIX_VP * GE_MATRIX_M * i_position;
}

#Type fragment
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 o_color;
struct VertexOutput
{
	vec4 color;
	vec4 normal;
};
layout (location = 0) in VertexOutput IN;

void main()
{
	o_color = IN.color;
}
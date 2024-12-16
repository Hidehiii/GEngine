#Name PresentImGui
#Cull Back
#Blend None
#DepthWrite On
#DepthTest LEqual
#Properties

sampler2D GE_PRESENT_IMGUI;

#EndProperties
#Type vertex
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
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
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 o_color;
struct VertexOutput
{
	vec2 uv;
};
layout (location = 0) in VertexOutput IN;
layout (binding = 40) uniform sampler2D GE_PRESENT_IMGUI;



void main()
{
	vec2 newUV = TransformUV(IN.uv);
	vec4 imgui = texture(GE_PRESENT_IMGUI, newUV);
	o_color = imgui;
	o_color.rgb = Vec3ToSrgb(o_color.rgb);
}
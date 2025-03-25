
#Name Present

#Properties
sampler2D GE_PRESENT_FRAME_BUFFER;
sampler2D GE_PRESENT_IMGUI;
float f;
vector v;
#EndProperties
#Type vertex
#include "Assets/Shaders/Core/Core.glsl"
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
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 o_color;
struct VertexOutput
{
	vec2 uv;
	
};
layout (location = 0) in VertexOutput IN;
layout (binding = GE_BINDING_START + 0) uniform sampler2D GE_PRESENT_FRAME_BUFFER;
layout (binding = GE_BINDING_START + 1) uniform sampler2D GE_PRESENT_IMGUI;

void main()
{
	vec2 newUV = TransformUV(IN.uv);
	vec4 imgui = texture(GE_PRESENT_IMGUI, newUV);
	float rate = step(imgui.r, 0.0001f);
	o_color = texture(GE_PRESENT_FRAME_BUFFER, newUV) * (rate) + imgui * (1 - rate);
	o_color.rgb = Vec3ToSrgb(o_color.rgb);
}
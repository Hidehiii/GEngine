
#Name CopyColorDepth

#Properties
sampler2D GE_PREVIOUS_COLOR;
sampler2D GE_PREVIOUS_DEPTH;
#EndProperties
#Type vertex
#version 450 core
layout(location = 0) in vec4 i_Position;
layout(location = 1) in vec2 i_UV;
struct VertexOutput
{
	vec2 UV;
};
layout (location = 0) out VertexOutput OUT;
void main()
{
	OUT.UV = i_UV;
	gl_Position =  i_Position;
}

#Type fragment
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 o_Color;
struct VertexOutput
{
	vec2 UV;
};
layout (location = 0) in VertexOutput IN;
layout (binding = GE_BINDING_START + 0) uniform sampler2D GE_PREVIOUS_COLOR;
layout (binding = GE_BINDING_START + 1) uniform sampler2D GE_PREVIOUS_DEPTH;

void main()
{
	vec2 newUV = TransformUV(IN.UV);
	o_Color = texture(GE_PREVIOUS_COLOR, newUV);
	gl_FragDepth = texture(GE_PREVIOUS_DEPTH, newUV).r;
}
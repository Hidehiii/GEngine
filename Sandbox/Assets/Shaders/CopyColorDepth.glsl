
#Name CopyColorDepth

#Properties
sampler2D _SceneColor;
sampler2D _SceneDepth;
#EndProperties

#BeginBlock CopyColorDepthVS
#include"Assets/Shaders/Core/Core.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_UV;

struct v2f
{
	vec2 UV;	
};

layout(location = 0) out v2f OUT;

void main()
{
	OUT.UV = in_UV;
	gl_Position = in_Position;
}
#EndBlock

#BeginBlock CopyColorDepthFS
#include"Assets/Shaders/Core/Core.glsl"

layout(location = 0) out vec4 out_Color;

struct v2f
{
	vec2 UV;	
};

layout(location = 0) in v2f IN;

layout(binding = GE_BINDING_START + 0) uniform sampler2D _SceneColor;
layout(binding = GE_BINDING_START + 1) uniform sampler2D _SceneDepth;

void main()
{
	vec2 newUV = TransformUV(IN.UV);
	out_Color = texture(_SceneColor, newUV);
	gl_FragDepth = texture(_SceneDepth, newUV).r;
}
#EndBlock

##Pass 1
Vertex CopyColorDepthVS
Fragment CopyColorDepthFS
##EndPass
#Name Present

#Properties

sampler2D _SceneColor;
sampler2D _ImguiColor;

#EndProperties

#BeginBlock VS
#version 450 core
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

#BeginBlock FS
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 out_Color;

struct v2f
{
	vec2 UV;
};

layout(location = 0) in v2f IN;

layout(binding = GE_BINDING_START + 0) uniform sampler2D _SceneColor;
layout(binding = GE_BINDING_START + 1) uniform sampler2D _ImguiColor;

void main()
{
	vec2 newUV = TransformUV(IN.UV);
	vec4 imgui = texture(_ImguiColor, newUV);
	float rate = step(imgui.r, 0.0001f);
	o_color = texture(_SceneColor, newUV) * (rate) + imgui * (1 - rate);
	o_color.rgb = Vec3ToSrgb(o_color.rgb);
}
#EndBlock

##Pass 1
Blend Add SrcAlpha OneMinusSrcAlpha
Vertex VS
Fragment FS 
##EndPass

#Name OITRender

#Properties

StorageImage2D HeadIndexImage;
StorageBuffer LinkedListSBO;
sampler2D _SceneColor;

#EndProperties

#BeginBlock RenderVS
#include "Assets/Shaders/Core/Core.glsl"

layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec2 in_UV;

struct v2f
{
	vec4 Position;
	vec2 UV;
};

layout(location = 0) out v2f OUT;

void main()
{
	OUT.UV = in_UV;
	gl_Position =  (in_Position);
	OUT.Position = gl_Position;
}
#EndBlock

#BeginBlock RenderFS
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 out_Color;

layout (binding = GE_BINDING_START + 0, r32ui) uniform uimage2D HeadIndexImage;

struct Node
{
	vec4 color;
	float depth;
	uint next;	
};
layout (binding = GE_BINDING_START + 1) buffer LinkedListSBO
{
	Node nodes[];	
};
layout (binding = GE_BINDING_START + 2) uniform sampler2D _SceneColor;

struct v2f
{
	vec4 Position;
	vec2 UV;
};
layout(location = 0) in v2f IN;
void main()
{
	Node fragments[4];
    int count = 0;

    uint nodeIdx = imageLoad(HeadIndexImage, ivec2(gl_FragCoord.xy)).r;
	//数据读取
    while (nodeIdx != 0xffffffff && count < 4)
    {
        fragments[count] = nodes[nodeIdx];
        nodes[nodeIdx].next = -1;
        nodeIdx = fragments[count].next;
        ++count;
    }
    
    // 使用插入排序，深度值从大到小
    for (uint i = 1; i < count; ++i)
    {
        Node insert = fragments[i];
        uint j = i;
        while (j > 0 && insert.depth > fragments[j - 1].depth)
        {
            fragments[j] = fragments[j-1];
            --j;
        }
        fragments[j] = insert;
    }

    // 混合处理
    vec2 newUV = TransformUV(IN.UV);
    vec4 color = texture(_SceneColor, newUV);  //底图
    for (int i = 0; i < count; ++i)
    {
        color = mix(color, fragments[i].color, fragments[i].color.a);
    }
    out_Color = color;
    imageAtomicExchange(HeadIndexImage, ivec2(gl_FragCoord.xy), -1);
}
#EndBlock


##Pass 1
DepthWrite Off
Blend Add SrcAlpha OneMinusSrcAlpha
Vertex RenderVS
Fragment RenderFS
##EndPass
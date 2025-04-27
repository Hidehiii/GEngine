
#Name OIT

#Properties

StorageImage2D HeadIndexImage;
StorageBuffer LinkedListSBO;
StorageBuffer GeometrySBO;
sampler2D _SceneColor;

#EndProperties

#BeginBlock WriteVS
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) in vec4 in_Position;
layout(location = 1) in vec4 in_Color;

struct v2f
{
	vec4 Position;
	vec4 Color;
};

layout(location = 0) out v2f OUT;

void main()
{
	OUT.Color = in_Color;
	gl_Position =  GE_MATRIX_VP * (in_Position);
	OUT.Position = gl_Position;
}
#EndBlock

#BeginBlock WriteFS
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"

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
layout (binding = GE_BINDING_START + 2) buffer GeometrySBO
{
	uint count;
    uint maxNodeCount;
};

struct v2f
{
	vec4 Position;
	vec4 Color;
};

layout(location = 0) in v2f IN;
void main()
{
	uint nodeIdx = atomicAdd(count, 1);
	if (nodeIdx < maxNodeCount)
    {
        // 交换新的链表头索引和以前的链表头
        uint prevHeadIdx = imageAtomicExchange(HeadIndexImage, ivec2(gl_FragCoord.xy), nodeIdx);

        // 存储数据至链表
        nodes[nodeIdx].color = IN.Color;
        nodes[nodeIdx].depth = gl_FragCoord.z;
        nodes[nodeIdx].next = prevHeadIdx;
    }
}
#EndBlock

#BeginBlock RenderVS
#version 450 core
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
	OUT.position = gl_Position;
}
#EndBlock

#BeginBlock RenderFS
#version 450 core
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
layout (binding = GE_BINDING_START + 3) uniform sampler2D _SceneColor;

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
    vec2 newUV = TransformUV(IN.uv);
    vec4 color = texture(_SceneColor, newUV);  //底图
    for (int i = 0; i < count; ++i)
    {
        color = mix(color, fragments[i].color, fragments[i].color.a);
    }
    o_color = color;
    imageAtomicExchange(HeadIndexImage, ivec2(gl_FragCoord.xy), -1);
}
#EndBlock

#BeginBlock ResetCs
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout (binding = GE_BINDING_START + 2) buffer GeometrySBO
{
	uint count;
    uint maxNodeCount;
};

void main()
{
	atomicExchange(count, 0);
	uint c = uint(GE_SCREEN_SIZE.x * GE_SCREEN_SIZE.y * 4);
	atomicExchange(maxNodeCount, c);
}
#EndBlock

##Pass Write
Cull None
Vertex WriteVS
Fragment WriteFS
##EndPass

##Pass Render
DepthWrite Off
Blend Add SrcAlpha OneMinusSrcAlpha
Vertex RenderVS
Fragment RenderFS
##EndPass

##Pass Reset
Compute ResetCs
##EndPass
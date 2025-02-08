
#Name OITPrepare
#Cull None
#DepthWrite On
#Blend None
#Properties

StorageImage2D headIndexImage;
StorageBuffer LinkedListSBO;
StorageBuffer GeometrySBO;
#EndProperties
#Type vertex
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
struct VertexOutput
{
	vec4 position;
	vec4 color;
};
layout (location = 0) out VertexOutput OUT;
void main()
{
	OUT.color = i_color;
	gl_Position =  GE_MATRIX_VP * (i_position);
	OUT.position = gl_Position;
}

#Type fragment
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"
layout (binding = GE_BINDING_START, r32ui) uniform uimage2D headIndexImage;
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
struct VertexOutput
{
	vec4 position;
	vec4 color;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	uint nodeIdx = atomicAdd(count, 1);
	if (nodeIdx < maxNodeCount)
    {
        // 交换新的链表头索引和以前的链表头
        uint prevHeadIdx = imageAtomicExchange(headIndexImage, ivec2(gl_FragCoord.xy), nodeIdx);

        // 存储数据至链表
        nodes[nodeIdx].color = IN.color;
        nodes[nodeIdx].depth = gl_FragCoord.z;
        nodes[nodeIdx].next = prevHeadIdx;
    }
}
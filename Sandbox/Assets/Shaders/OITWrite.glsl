
#Name OITWrite

#Properties

StorageImage2D HeadIndexImage;
StorageBuffer LinkedListSBO;
StorageBuffer GeometrySBO;

#EndProperties

#BeginBlock WriteVS
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

#BeginBlock ResetCs
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

##Pass Reset
Compute ResetCs
##EndPass
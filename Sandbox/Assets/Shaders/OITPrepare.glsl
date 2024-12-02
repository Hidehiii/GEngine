
#Name OITPrepare

#Properties

StorageImage2D headIndexImage;
StorageBuffer LinkedListSBO;
StorageBuffer GeometrySBO;

#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
layout(std140, binding = 1) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
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
layout(location = 0) out vec4 o_color;
layout(std140, binding = 2) uniform TIME
{
	vec4 GE_TIME;
};

layout (binding = 20, r32ui) uniform uimage2D headIndexImage;
struct Node
{
	vec4 color;
	float depth;
	uint next;	
};
layout (binding = 21) buffer LinkedListSBO
{
	Node nodes[];	
};
layout (binding = 22) buffer GeometrySBO
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
    o_color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
}
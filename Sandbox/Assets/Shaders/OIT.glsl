
#Name OIT
#DepthWrite Off
#DepthTest LEqual
#Blend Add SrcAlpha OneMinusSrcAlpha
#Properties
StorageImage2D headIndexImage;
StorageBuffer LinkedListSBO;
sampler2D OpaqueColor;
#EndProperties
#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
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
	vec2 uv;
};
layout (location = 0) out VertexOutput OUT;
void main()
{
	OUT.uv = i_uv;
	gl_Position =  (i_position);
	OUT.position = gl_Position;
}

#Type fragment
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 o_color;
layout (binding = GE_BINDING_START + 0, r32ui) uniform uimage2D headIndexImage;
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
layout (binding = GE_BINDING_START + 2) uniform sampler2D OpaqueColor;
struct VertexOutput
{
	vec4 position;
	vec2 uv;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	Node fragments[4];
    int count = 0;

    uint nodeIdx = imageLoad(headIndexImage, ivec2(gl_FragCoord.xy)).r;
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
    vec4 color = texture(OpaqueColor, newUV);  //底图
    for (int i = 0; i < count; ++i)
    {
        color = mix(color, fragments[i].color, fragments[i].color.a);
    }
    o_color = color;
    imageAtomicExchange(headIndexImage, ivec2(gl_FragCoord.xy), -1);
}
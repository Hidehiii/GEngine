
#Name ComputeTest
#Cull None
#DepthWrite On
#Blend None
#Properties
StorageImage2D computeTestImage;
StorageBuffer GeometrySBO;

#EndProperties
#Type compute
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = GE_BINDING_START, rgba16f) uniform image2D computeTestImage;
layout (binding = GE_BINDING_START + 1) buffer GeometrySBO
{
	uint count;
    uint maxNodeCount;
};
void main()
{
	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
	ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
	
    value.x = float(texelCoord.x)/(gl_NumWorkGroups.x);
    value.y = float(texelCoord.y)/(gl_NumWorkGroups.y);
	imageStore(computeTestImage, texelCoord, value);

	atomicExchange(count, 0);
	uint c = uint(GE_SCREEN_SIZE.x * GE_SCREEN_SIZE.y * 4);
	atomicExchange(maxNodeCount, c);
}

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
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 2) uniform TIME
{
	vec4 GE_TIME;
};
layout(std140, binding = 5) uniform SCREEN
{
	vec4 GE_SCREEN_SIZE;	
};
layout (binding = 40, rgba16f) uniform image2D computeTestImage;
layout (binding = 41) buffer GeometrySBO
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
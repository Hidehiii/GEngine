
#Name instance

#Properties

StorageImage2D testImage;

#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec4 i_offset;
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
	gl_Position =  GE_MATRIX_VP * (i_position + i_offset);
	OUT.position = gl_Position;
}

#Type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
layout(std140, binding = 2) uniform TIME
{
	float GE_TIME;
};

layout (binding = 10, rgba32f) uniform image2D testImage;
struct VertexOutput
{
	vec4 position;
	vec2 uv;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	o_color = vec4(IN.uv, 0.0f, 1.0f);


	imageStore(testImage, ivec2(gl_FragCoord.xy), vec4(gl_FragCoord.x, gl_FragCoord.y, 1.0f, 1.0f));
	vec4 col = imageLoad(testImage, ivec2(gl_FragCoord.xy));
	o_color.b = col.r;
}
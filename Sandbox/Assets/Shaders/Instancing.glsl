
#Name instance

#Properties

StorageImage2D testImage;
Sampler2D testTex;
StorageBuffer testBuffer;

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
	vec4 GE_TIME;
};

layout (binding = 20, rgba32f) uniform image2D testImage;
layout (binding = 21) uniform sampler2D testTex;
struct SSBOData
{
	vec4 color;
	float alpha;	
};
layout (binding = 22) buffer testBuffer
{
	SSBOData ssbo[];	
};
struct VertexOutput
{
	vec4 position;
	vec2 uv;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	o_color = texture(testTex, IN.uv);

	ivec2 size = imageSize(testImage);
	vec4 col = imageLoad(testImage, ivec2(0, 0));
	//imageStore(testImage, ivec2(0, 0) , vec4(fract(GE_TIME.y), fract(GE_TIME.y), 0.0f, 1.0f));


	o_color.rgb = col.rgb;

	o_color.rgb = ssbo[0].color.rgb;
}

#Name Present

#Properties
sampler2D GE_PRESENT_FRAME_BUFFER;
sampler2D GE_PRESENT_IMGUI;

#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
struct VertexOutput
{
	vec2 uv;
};
layout (location = 0) out VertexOutput OUT;
void main()
{
	OUT.uv = i_uv;
	gl_Position =  i_position;
}

#Type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
struct VertexOutput
{
	vec2 uv;
	
};
layout (location = 0) in VertexOutput IN;
layout (binding = 10) uniform sampler2D GE_PRESENT_FRAME_BUFFER;
layout (binding = 11) uniform sampler2D GE_PRESENT_IMGUI;


float floatToSrgb(float value) {
    const float inv_12_92 = 0.0773993808;
    return value <= 0.04045
       ? value * inv_12_92 
       : pow((value + 0.055) / 1.055, 2.4);
}
vec3 vec3ToSrgb(vec3 value) {
    return vec3(floatToSrgb(value.x), floatToSrgb(value.y), floatToSrgb(value.z));
}

void main()
{
	vec2 newUV = IN.uv;
#if GE_ATTACHMENT_UV_STARTS_AT_TOP
	newUV.y = 1- newUV.y;
#endif
	vec4 imgui = texture(GE_PRESENT_IMGUI, newUV);
	float rate = step(imgui.r, 0.0001f);
	o_color = texture(GE_PRESENT_FRAME_BUFFER, newUV) * (rate) + imgui * (1 - rate);
	o_color.rgb = vec3ToSrgb(o_color.rgb);
}
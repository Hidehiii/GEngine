
#Name AlphaTestShader
#Blend Alpha SrcAlpha OneMinusSrcAlpha
#DepthWrite Off
#DepthTest On
#Properties

p0: color;

p1: vector
p2: float


#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec4 i_normal;

layout(std140, binding = 1) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
layout(std140, binding = 2) uniform TIME
{
	float GE_TIME;
};
layout(std140, binding = 3) uniform MODEL
{
	mat4 GE_MATRIX_M;
};
layout(std140, binding = 4) uniform LIGHT
{
	vec4 GE_MAIN_LIGHT_DIRECTION;
	vec4 GE_MAIN_LIGHT_COLOR;
};
struct VertexOutput
{
	vec4 color;
	vec4 normal;
};
layout (set = 1, location = 0) out VertexOutput OUT;
void main()
{
	OUT.color = i_color;
	OUT.normal = normalize(GE_MATRIX_M * i_normal);
	gl_Position = GE_MATRIX_VP * GE_MATRIX_M * i_position;
}

#Type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
layout(std140, binding = 1) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
layout(std140, binding = 0) uniform MATERIAL
{
	vec4 p0;
	vec4 p1;
	float p2;
};
struct VertexOutput
{
	vec4 color;
	vec4 normal;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	vec4 c = IN.normal;
	c.xyz = p0.xyz;
	o_color = vec4(c.x, c.y, c.z, p2);
}
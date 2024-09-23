
#Name Default
#Blend None
#DepthMask On
#DepthTest On
#Properties

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
layout (location = 0) in VertexOutput IN;
void main()
{
	vec4 col = (dot(normalize(IN.normal.xyz), -normalize(GE_MAIN_LIGHT_DIRECTION.xyz)) * 0.5 + 0.5) * GE_MAIN_LIGHT_COLOR;
	o_color = vec4(col.x, col.y, col.z, 1.0);
}
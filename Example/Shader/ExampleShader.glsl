#Name ExampleShader
#Blend Alpha SrcAlpha OneMinusSrcAlpha
#DepthWrite On
#DepthTest On
#Properties

color prop1;
float prop2;
int prop3;
vector prop4;

#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec4 i_normal;

layout(std140, binding = 0) uniform MATERIAL
{
	alignas(16) vec4 prop1;
	alignas(16) float prop2;
	alignas(16) int prop3;
	alignas(16) vec4 prop;
}
layout(std140, binding = 1) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
layout(std140, binding = 2) uniform TIME
{
	vec4 GE_TIME;
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
layout(std140, binding = 5) uniform SCREEN
{
	vec4 GE_SCREEN_SIZE;	
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

layout(std140, binding = 0) uniform MATERIAL
{
	alignas(16) vec4 prop1;
	alignas(16) float prop2;
	alignas(16) int prop3;
	alignas(16) vec4 prop;
}
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
layout(std140, binding = 5) uniform SCREEN
{
	vec4 GE_SCREEN_SIZE;	
};

struct VertexOutput
{
	vec4 color;
	vec4 normal;
};
layout (location = 0) in VertexOutput IN;

void main()
{
	o_color = IN.color;
}

#Name CubeMap
#DepthWrite Off
#Blend None
#Properties
SamplerCube TexCube;
#EndProperties
#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(std140, binding = 1) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
layout(std140, binding = 3) uniform MODEL
{
	mat4 GE_MATRIX_M;
};
struct VertexOutput
{
	vec4 position;
};
layout (location = 0) out VertexOutput OUT;
void main()
{
	mat4 view = mat4(
		GE_MATRIX_V[0][0], GE_MATRIX_V[0][1], GE_MATRIX_V[0][2], 0.0f,
		GE_MATRIX_V[1][0], GE_MATRIX_V[1][1], GE_MATRIX_V[1][2], 0.0f,
		GE_MATRIX_V[2][0], GE_MATRIX_V[2][1], GE_MATRIX_V[2][2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	vec4 pos = GE_MATRIX_P * view * GE_MATRIX_M * vec4(i_position.xyz, 1.0f);
	gl_Position =  pos;
	OUT.position = i_position;
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
layout(std140, binding = 2) uniform TIME
{
	vec4 GE_TIME;
};
layout (binding = 40) uniform samplerCube TexCube;
struct VertexOutput
{
	vec4 position;
};
layout (location = 0) in VertexOutput IN;
void main()
{

    vec3 uv = vec3(IN.position.x, IN.position.y, IN.position.z);
    o_color = texture(TexCube, uv);
}
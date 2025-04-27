
#Name CubeMap

#Properties

SamplerCube CubeMap;

#EndProperties

#BeginBlock CubeMapVS
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"

layout(location = 0) in vec4 in_Position;

struct v2f
{
	vec4 Position;	
};

layout(location = 0) out v2f OUT;

void main()
{
	mat4 view = mat4(
		GE_MATRIX_V[0][0], GE_MATRIX_V[0][1], GE_MATRIX_V[0][2], 0.0f,
		GE_MATRIX_V[1][0], GE_MATRIX_V[1][1], GE_MATRIX_V[1][2], 0.0f,
		GE_MATRIX_V[2][0], GE_MATRIX_V[2][1], GE_MATRIX_V[2][2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	vec4 pos = GE_MATRIX_P * view * GE_MATRIX_M * vec4(in_Position.xyz, 1.0f);
	gl_Position =  pos;
	OUT.Position = in_Position;
}

#EndBlock

#BeginBlock CubeMapFS
#version 450 core
#include "Assets/Shaders/Core/Core.glsl"

layout(location = 0) out vec4 out_Color;
layout(binding = GE_BINDING_START + 0) uniform samplerCube CubeMap;

struct v2f
{
	vec4 Position;	
};

layout(location = 0) in v2f IN;

void main()
{
	vec3 uv = vec3(IN.Position.x, IN.Position.y, IN.Position.z);
	out_Color = texture(CubeMap, uv);
}
#EndBlock

##Pass 1
DepthWrite Off
Vertex CubeMapVS
Fragment CubeMapFS
##EndPass
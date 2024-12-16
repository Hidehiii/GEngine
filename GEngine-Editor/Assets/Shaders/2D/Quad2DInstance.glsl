#Name Quad2DInstance
#Blend Add SrcAlpha OneMinusSrcAlpha
#DepthWrite Off
#DepthTest LEqual
#Cull Back
#Properties

sampler2D Tex_0;
sampler2D Tex_1;
sampler2D Tex_2;
sampler2D Tex_3;
sampler2D Tex_4;
sampler2D Tex_5;
sampler2D Tex_6;
sampler2D Tex_7;
#EndProperties
#Type vertex
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) in vec4 i_PositionOS;
layout(location = 1) in vec2 i_UV;
layout(location = 2) in vec2 i_Tiling;
layout(location = 3) in vec4 i_Color;
layout(location = 4) in int i_TexIndex;
layout(location = 5) in vec4 i_ModelMatrixRow_1;
layout(location = 6) in vec4 i_ModelMatrixRow_2;
layout(location = 7) in vec4 i_ModelMatrixRow_3;
layout(location = 8) in vec4 i_ModelMatrixRow_4;

struct VertexOutput
{
	vec2 UV;
	vec4 Color;
	vec2 Tiling;
};
layout (location = 0) out VertexOutput OUT;
layout (location = 3) flat out int v_TexIndex;
void main()
{
	OUT.UV = i_UV;
	OUT.Color = i_Color;
	OUT.Tiling = i_Tiling;
	v_TexIndex = i_TexIndex;
	mat4 modelMatrix = mat4(i_ModelMatrixRow_1,
							i_ModelMatrixRow_2,
							i_ModelMatrixRow_3,
							i_ModelMatrixRow_4);
	gl_Position = GE_MATRIX_VP * modelMatrix * i_PositionOS;
}

#Type fragment
#version 450 core
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 o_Color;
struct VertexOutput
{
	vec2 UV;
	vec4 Color;
	vec2 Tiling;
};
layout (location = 0) in VertexOutput IN;
layout (location = 3) in flat int v_TexIndex;
layout (binding = 40) uniform sampler2D Tex_0;
layout (binding = 41) uniform sampler2D Tex_1;
layout (binding = 42) uniform sampler2D Tex_2;
layout (binding = 43) uniform sampler2D Tex_3;
layout (binding = 44) uniform sampler2D Tex_4;
layout (binding = 45) uniform sampler2D Tex_5;
layout (binding = 46) uniform sampler2D Tex_6;
layout (binding = 47) uniform sampler2D Tex_7;
void main()
{
	o_Color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	switch(v_TexIndex)
	{
		case 0: o_Color = texture(Tex_0, IN.UV * IN.Tiling) * IN.Color; break;
		case 1: o_Color = texture(Tex_1, IN.UV * IN.Tiling) * IN.Color; break;
		case 2: o_Color = texture(Tex_2, IN.UV * IN.Tiling) * IN.Color; break;
		case 3: o_Color = texture(Tex_3, IN.UV * IN.Tiling) * IN.Color; break;
		case 4: o_Color = texture(Tex_4, IN.UV * IN.Tiling) * IN.Color; break;
		case 5: o_Color = texture(Tex_5, IN.UV * IN.Tiling) * IN.Color; break;
		case 6: o_Color = texture(Tex_6, IN.UV * IN.Tiling) * IN.Color; break;
		case 7: o_Color = texture(Tex_7, IN.UV * IN.Tiling) * IN.Color; break;
	}
}
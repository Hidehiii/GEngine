
#Name Default
#Blend None
#DepthWrite On
#DepthTest LEqual
#Properties
#EndProperties

#Type vertex
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) in vec4 i_PositionOS;
layout(location = 1) in vec4 i_Color;
layout(location = 2) in vec4 i_Normal;
layout(location = 3) in vec4 i_Tangent;
layout(location = 4) in vec2 i_UV0;
layout(location = 5) in vec2 i_UV1;
layout(location = 6) in vec2 i_UV2;
layout(location = 7) in vec2 i_UV3;
struct VertexOutput
{
	vec4 Color;
	vec4 Normal;
};
layout (location = 0) out VertexOutput OUT;
void main()
{
	OUT.Color = i_Color;
	OUT.Normal = normalize(GE_MATRIX_M * i_Normal);
	gl_Position = GE_MATRIX_VP * GE_MATRIX_M * i_PositionOS;
}

#Type fragment
#include"Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 o_Color;
struct VertexOutput
{
	vec4 Color;
	vec4 Normal;
};
layout (location = 0) in VertexOutput IN;
void main()
{
	vec4 col = (dot(normalize(IN.Normal.xyz), -normalize(GE_MAIN_LIGHT_DIRECTION.xyz)) * 0.5 + 0.5) * GE_MAIN_LIGHT_COLOR;
	o_Color = vec4(col.x, col.y, col.z, 1.0);
}
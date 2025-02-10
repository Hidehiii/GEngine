
#Name SperateTexture2DAndSampler
#DepthWrite Off
#Blend None
#Properties
Sampler2D tex;
#EndProperties
#Type vertex
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) in vec4 i_position;
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
#include "Assets/Shaders/Core/Core.glsl"
layout(location = 0) out vec4 o_color;
layout (binding = GE_BINDING_START + 0) uniform texture2D tex;
layout (binding = GE_BINDING_START + 1) uniform sampler samp;
struct VertexOutput
{
	vec4 position;
};
layout (location = 0) in VertexOutput IN;
void main()
{

    vec2 uv = vec2(IN.position.x, IN.position.y);
    o_color = texture(sampler2D(tex, samp), uv);
}
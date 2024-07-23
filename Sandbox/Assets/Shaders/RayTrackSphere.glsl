
#name Sphere3D

#type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec2 i_tiling;
layout(location = 3) in vec4 i_color;
layout(location = 4) in int i_texIndex;
layout(std140, binding = 0) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
struct VertexOutput
{
	vec2 uv;
	vec4 color;
	vec2 tiling;
	vec4 positionWS;
};
layout (location = 0) out VertexOutput i;
layout (location = 4) flat out int v_TexIndex;
void main()
{
	i.uv = i_uv;
	i.color = i_color;
	i.tiling = i_tiling;
	i.positionWS = i_position;
	v_TexIndex = i_texIndex;
	gl_Position = GE_MATRIX_VP * i_position;
}

#type fragment
#version 450 core
layout(location = 0) out vec4 o_color;
layout(std140, binding = 0) uniform CAMERA
{
	mat4 GE_MATRIX_V;
	mat4 GE_MATRIX_P;
	mat4 GE_MATRIX_VP;
	vec4 GE_CAMERA_POSITION;
};
layout(std140, binding = 1) uniform TIME
{
	float GE_TIME;
};
struct VertexOutput
{
	vec2 uv;
	vec4 color;
	vec2 tiling;
	vec4 positionWS;
};
layout (location = 0) in VertexOutput IN;
layout (location = 4) in flat int v_TexIndex;
layout (binding = 0) uniform sampler2D _Textures[32];

void main()
{
	int hit = 0;
	vec3 lightDir = vec3(1,1,1);
	vec3 normal;
	vec3 pos;
	float sphereR[] = { 2, 2, 100 };
	vec3 sphereOrigin[] = { vec3(-3,0,0), vec3(3,0,0), vec3(0, -102.5, 0)};
	vec4 sphereCol[] = { vec4(0.8,0.1,0.2,1), vec4(0.2,0.8,0.2,1), vec4(0.2,0.2,0.8,1)};

	vec4 finCol = vec4(0,0,0,1);
	int bounds = 5;
	float multiplater = 0.5f;


	vec3 castPos = GE_CAMERA_POSITION.xyz;
	vec3 dir = IN.positionWS.xyz - castPos;
	dir = normalize(dir);

	for(int k = 0;k < bounds;k++)
	{
		float closeDis = -1;
		int cloestIndex = -1;
	
		for(int i = 0;i < 3;i++)
		{
			float r = sphereR[i];
			vec3 sphereOri = sphereOrigin[i];
			// (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0
			// a = ray origin
			// b = ray direction
			// r = radius
			// t = hit distance
			vec3 castDir = (castPos - sphereOrigin[i]);
			float a = dot(dir, dir);
			float b = 2 * dot(castDir, dir);
			float c = dot(castDir, castDir) - r * r;

			// b^2 - 4ac
			float d = b * b - 4 * a * c;

			if(d >= 0)
			{
				float t0 = (-b - sqrt(d)) / (2 * a);
				t0 = min(t0,(-b + sqrt(d)) / (2 * a));
				if(cloestIndex == -1)
				{
					pos = castPos + dir * t0;
					normal = pos - sphereOri;
					normal = normalize(normal);
					cloestIndex = i;
					closeDis = t0;
					hit = 1;
				}
				else if(t0  < closeDis)
				{
					pos = castPos + dir * t0;
					normal = pos - sphereOri;
					normal = normalize(normal);
					cloestIndex = i;
					closeDis = t0;
					hit = 1;
				}
			}
		}

		if(cloestIndex != -1)
		{
			float Lightintensity = max(0,dot(lightDir, normal));
			finCol += sphereCol[cloestIndex] * Lightintensity * multiplater;
			castPos = pos + normal*0.000001f;
			dir = reflect(-dir,normal);
			dir = normalize(dir);
			multiplater *= multiplater;
		}
		else
		{
			break;
		}
	}
	
	o_color = finCol;
	if(hit == 0)
	{
		discard;
	}
}
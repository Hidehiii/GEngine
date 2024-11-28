
#Name NormalWS3D

#Properties

vector p0;
vector p1;

#Type vertex
#version 450 core
layout(location = 0) in vec4 i_position;
layout(location = 1) in vec2 i_uv;
layout(location = 2) in vec2 i_tiling;
layout(location = 3) in vec4 i_color;
layout(location = 4) in int i_texIndex;
layout(std140, binding = 1) uniform CAMERA
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

struct Sphere
{
	vec3 origin;
	float radius;
	vec4 color;
};

struct Ray
{
	vec3 origin;
	vec3 direction;
};

struct Hit
{
	int isHit;
	vec3 position;
	vec3 normal;
	Sphere sphere;
};


void main()
{
	vec3 lightDir = vec3(1,1,1);
	vec3 normal;
	vec3 pos;
	

	Sphere _1;
	_1.origin = vec3(-3,0,0);
	_1.radius = 2;
	_1.color = vec4(0.8,0.1,0.2,1);

	Sphere _2;
	_2.origin = vec3(3,0,0);
	_2.radius = 2;
	_2.color = vec4(0.2,0.8,0.2,1);

	Sphere _3;
	_3.origin = vec3(0, -102.5, 0);
	_3.radius = 100;
	_3.color = vec4(0.2,0.2,0.8,1);

	Sphere spheres[] = { _1, _2, _3};

	vec4 finCol = vec4(0,0,0,1);
	int bounds = 8;
	float multiplater = 0.5f;


	Ray ray;
	ray.origin = GE_CAMERA_POSITION.xyz;
	ray.direction = IN.positionWS.xyz - ray.origin;
	ray.direction = normalize(ray.direction);

	Hit hit;
	hit.isHit = 0;
	float closeDis = -1;
	int cloestIndex = -1;
	for(int k = 0;k < bounds;k++)
	{
		cloestIndex = -1;
		closeDis = -1;
		for(int i = 0;i < 3;i++)
		{
			float r = spheres[i].radius;
			vec3 sphereOri = spheres[i].origin;
			// (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0
			// a = ray origin
			// b = ray direction
			// r = radius
			// t = hit distance
			vec3 castDir = (ray.origin - spheres[i].origin);
			float a = dot(ray.direction, ray.direction);
			float b = 2 * dot(castDir, ray.direction);
			float c = dot(castDir, castDir) - r * r;

			// b^2 - 4ac
			float d = b * b - 4 * a * c;

			if(d >= 0)
			{
				float t0 = (-b - sqrt(d)) / (2 * a);
				t0 = min(t0,(-b + sqrt(d)) / (2 * a));
				if(cloestIndex == -1)
				{
					pos = ray.origin + ray.direction * t0;
					normal = pos - spheres[i].origin;
					normal = normalize(normal);
					cloestIndex = i;
					closeDis = t0;
					hit.position = pos;
					hit.normal = normal;
					hit.isHit = 1;
					hit.sphere = spheres[i];
				}
				else if(t0  < closeDis)
				{
					pos = ray.origin + ray.direction * t0;
					normal = pos - spheres[i].origin;
					normal = normalize(normal);
					cloestIndex = i;
					closeDis = t0;
					hit.position = pos;
					hit.normal = normal;
					hit.isHit = 1;
					hit.sphere = spheres[i];
				}
			}
		}

		if(cloestIndex != -1)
		{
			float Lightintensity = max(0,dot(lightDir, normal));
			finCol += spheres[cloestIndex].color * Lightintensity * multiplater;
			ray.origin = pos + normal*0.000001f;
			ray.direction = reflect(-ray.direction,normal);
			ray.direction = normalize(ray.direction);
			multiplater *= multiplater;
		}
		else
		{
			break;
		}
	}
	
	o_color = finCol;
	if(hit.isHit != 1)
	{
		discard;
	}
}
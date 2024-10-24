#include "RayTracing.h"



static uint32_t ConvertToRGBA(GEngine::Vector4 col)
{
	uint8_t r = (uint8_t)(GEngine::Math::Clamp(col.x, 0.0f, 1.0f) * 255);
	uint8_t g = (uint8_t)(GEngine::Math::Clamp(col.y, 0.0f, 1.0f) * 255);
	uint8_t b = (uint8_t)(GEngine::Math::Clamp(col.z, 0.0f, 1.0f) * 255);
	uint8_t a = (uint8_t)(GEngine::Math::Clamp(col.w, 0.0f, 1.0f) * 255);
	return (a << 24) | (b << 16) | (g << 8) | r;
}

static uint32_t PCG_Hash(uint32_t input)
{
	uint32_t state = input * 747796405u + 2891336453u;
	uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

static float RandomFloat(uint32_t& input)
{
	return (float)PCG_Hash(input) / (float)std::numeric_limits<uint32_t>::max();
}

static GEngine::Vector3 RandomInUnitSphere(uint32_t& input)
{
	GEngine::Vector3 p;
	p.x = RandomFloat(input) * 2.0f - 1.0f;
	p.y = RandomFloat(input) * 2.0f - 1.0f;
	p.z = RandomFloat(input) * 2.0f - 1.0f;
	p = GEngine::Math::Normalized( p);
	return p;
}

void RayTracing::Init()
{
	srand(time(0));
	m_Distribution = std::uniform_real_distribution<float>(-1.0, 1.0);
	m_Texture = GEngine::Texture2D::Create(m_ScreenSize.x, m_ScreenSize.y);
	m_Texture->SetData(m_TextureData.data(), m_TextureData.size() * 4);

	m_Spheres.emplace_back(GEngine::Vector3(-23.0f, 10.0f, -20.0f), 10.0f, GEngine::Vector3(0.2f, 0.2f, 0.4f), 0.25f, 0.0f, GEngine::Vector3(0.4f, 0.3f, 0.2f), 10.0f);
	m_Spheres.emplace_back(GEngine::Vector3(23.0f, 10.0f, -20.0f), 10.0f, GEngine::Vector3(0.2f, 0.2f, 0.4f), 0.25f, 0.0f, GEngine::Vector3(0.1f, 0.3f, 0.4f), 10.0f);
	m_Spheres.emplace_back(GEngine::Vector3(0.0f, 0.0f, 0.0f), 1.0f, GEngine::Vector3(0.8f, 0.7f, 0.6f), 0.1f, 0.0f);
	m_Spheres.emplace_back(GEngine::Vector3(0.0f, 0.0f, 2.0f), 0.5f, GEngine::Vector3(0.4f, 0.4f, 0.6f), 0.1f, 0.0f, GEngine::Vector3(0.1f, 0.5f, 0.3f), 5.0f);
	m_Spheres.emplace_back(GEngine::Vector3(2.0f, 0.0f, 0.0f), 1.0f, GEngine::Vector3(0.8f, 0.3f, 0.7f), 0.1f, 0.0f);
	m_Spheres.emplace_back(GEngine::Vector3(-2.0f, 2.0f, 0.0f), 2.0f, GEngine::Vector3(0.4f, 0.1f, 0.3f), 0.1f, 0.0f);
	m_Spheres.emplace_back(GEngine::Vector3(0.0f, -101.0f, 0.0f), 100.0f, GEngine::Vector3(0.2f, 0.25f, 0.75f), 0.5f, 0.0f);

	for(uint32_t i = 0; i < m_ScreenSize.x; i++)
	{
		m_PixelsInOneLine.at(i) = i;
	}
}

void RayTracing::OnUpdate(GEngine::Editor::EditorCamera& camera)
{
	if (camera.GetPosition() != m_CameraPosition)
	{
		m_CameraPosition = camera.GetPosition();
		m_FrameCount = 1;
		std::for_each(std::execution::par, m_PixelsInOneLine.begin(), m_PixelsInOneLine.end(),
			[this, &camera](uint32_t y)
			{
				std::for_each(std::execution::par, m_PixelsInOneLine.begin(), m_PixelsInOneLine.end(),
				[this, y, &camera](uint32_t x)
					{
						GEngine::Vector2 uv = GEngine::Vector2(float(x) / m_ScreenSize.x, float(y) / m_ScreenSize.y);
						uv = uv * 2.0f - 1.0f;
						GEngine::Vector4 target = GEngine::Math::Inverse(camera.GetProjectionMatrix()) * GEngine::Vector4(uv, 1.0f, 1.0f);
						m_RayDir[x + y * m_ScreenSize.x] = { GEngine::Math::Inverse(camera.GetViewMatrix()) * GEngine::Vector4(GEngine::Math::Normalized(GEngine::Vector3(target) / target.w), 0.0f) };

						m_Accumulation[x + y * m_ScreenSize.x] = GEngine::Vector4();
					}
				);

			}
		);
	}
	std::for_each(std::execution::par, m_PixelsInOneLine.begin(), m_PixelsInOneLine.end(),
		[this, &camera](uint32_t y)
		{
			std::for_each(std::execution::par, m_PixelsInOneLine.begin(), m_PixelsInOneLine.end(),
			[this, y, &camera](uint32_t x)
				{
					m_Accumulation[x + y * m_ScreenSize.x] += PerPixelRayTrace(x, y, camera);

					GEngine::Vector4 color = m_Accumulation[x + y * m_ScreenSize.x] / (float)m_FrameCount;
					m_TextureData[x + y * m_ScreenSize.x] = ConvertToRGBA(color);

				}
			);
			
		}
	);
	m_FrameCount++;
	m_Texture->SetData(m_TextureData.data(), m_TextureData.size() * 4);
	GEngine::Renderer2D::DrawQuad(GEngine::Vector3(), GEngine::Vector3(), GEngine::Vector3(8, 8, 1), GEngine::Vector4(1.0f), m_Texture);
}

GEngine::Vector4 RayTracing::PerPixelRayTrace(uint32_t x, uint32_t y, GEngine::Editor::EditorCamera& camera)
{
	Ray ray;
	ray.origin = camera.GetPosition();
	ray.direction = m_RayDir[(x + y * m_ScreenSize.x)];

	GEngine::Vector3 light = { 0.0f, 0.0f, 0.0f };
	GEngine::Vector3 multiplier = { 1.0f, 1.0f, 1.0f };

	uint32_t seed = x + y * m_ScreenSize.x;
	seed *= m_FrameCount;
	
	HitRecord hitRecord;
	for (int i = 0; i < m_Bounce; i++)
	{
		seed += i;

		hitRecord = RayTrace(ray);	
		
		if (hitRecord.hit == false)
		{
			GEngine::Vector3 skyColor = { 0.0f, 0.0f, 0.0f };
			light += skyColor * multiplier;
			break;
		}
		
		multiplier *= hitRecord.color;
		//light += hitRecord.color * multiplier;
		//multiplier *= 0.5f;
		light += hitRecord.emissionColor * hitRecord.emissionIntensity;

		ray.origin = hitRecord.position + (hitRecord.normal * 0.0001f);
		ray.direction = GEngine::Math::Reflect(ray.direction,
			(hitRecord.normal + hitRecord.roughness * GEngine::Math::Normalized(GEngine::Vector3(m_Distribution(m_Engine), m_Distribution(m_Engine), m_Distribution(m_Engine)))));
		//ray.direction = (hitRecord.normal + GEngine::Vector3(m_Distribution(m_Engine), m_Distribution(m_Engine), m_Distribution(m_Engine)).Normalized()).Normalized();
		//ray.direction = (hitRecord.normal + RandomInUnitSphere(seed)).Normalized();
	}
	
	return { light, 1.0f };
}

HitRecord RayTracing::RayTrace(Ray& ray)
{
	float cloestDis = std::numeric_limits<float>::max();
	HitRecord hitRecord;
	hitRecord.hit = false;
	for (auto& sphere : m_Spheres)
	{
		GEngine::Vector3 sphereCenter = ray.origin - sphere.position;

		float a = GEngine::Math::Dot(ray.direction, ray.direction);
		float b = 2.0f * GEngine::Math::Dot(sphereCenter, ray.direction);
		float c = GEngine::Math::Dot(sphereCenter, sphereCenter) - sphere.radius * sphere.radius;

		float discriminant = b * b - 4.0f * a * c;

		if (discriminant >= 0)
		{
			float t1 = (-b - GEngine::Math::Sqrt(discriminant)) / (2.0f * a);
			if(cloestDis > t1 && t1 >0)
			{
				cloestDis = t1;
				hitRecord.hit = true;
				GEngine::Vector3 hitPoint = ray.origin + ray.direction * t1;
				hitRecord.position = hitPoint;

				//GEngine::Vector3 normal = ((hitPoint).Normalized());
				GEngine::Vector3 normal = GEngine::Math::Normalized((hitPoint - sphere.position));
				hitRecord.normal = normal;
				hitRecord.color = sphere.color;
				hitRecord.roughness = sphere.roughness;
				hitRecord.metalness = sphere.metalness;
				hitRecord.emissionIntensity = sphere.emissionIntensity;
				hitRecord.emissionColor = sphere.emissionColor;
			}
		}
	}
	return hitRecord;
}


#pragma once
#include "GEngine.h"
#include <random>
#include <thread>
#include <execution>


struct Ray
{
	GEngine::Vector3 origin;
	GEngine::Vector3 direction;
};


struct Sphere
{
	GEngine::Vector3 position = { 0.0f };
	float radius;

	GEngine::Vector3 color = { 0.0f };
	float roughness = 0.0f;
	float metalness = 0.0f;

	GEngine::Vector3 emissionColor = { 0.0f };
	float emissionIntensity = 0.0f;

	Sphere() = default;
	Sphere(GEngine::Vector3 p, float r, GEngine::Vector3 c, float ro, float me, GEngine::Vector3 emcol, float empow)
		: position(p), radius(r), color(c), roughness(ro), metalness(me), emissionColor(emcol), emissionIntensity(empow) {}
	Sphere(GEngine::Vector3 p, float r, GEngine::Vector3 c, float ro, float me)
		: position(p), radius(r), color(c), roughness(ro), metalness(me){}
	Sphere(GEngine::Vector3 p, float r, GEngine::Vector3 c)
		: position(p), radius(r), color(c) {}
};

struct HitRecord
{
	GEngine::Vector3 position;
	GEngine::Vector3 normal;
	GEngine::Vector3 color;
	float roughness = 0.0f;
	float metalness = 0.0f;

	GEngine::Vector3 emissionColor = { 0.0f };
	float emissionIntensity = 0.0f;
	bool hit = false;
};

class RayTracing
{
public:
	RayTracing() = default;
	void Init();
	void OnUpdate(GEngine::Editor::EditorCamera& camera);
public:
private:
	GEngine::Vector4 PerPixelRayTrace(uint32_t x, uint32_t y, GEngine::Editor::EditorCamera& camera);
	HitRecord RayTrace(Ray& ray);
private:
	GEngine::Vector2 m_ScreenSize = { 800, 800 };
	std::vector<uint32_t> m_PixelsInOneLine = std::vector<uint32_t>(m_ScreenSize.value.x, 0);
	std::vector<uint32_t> m_TextureData = std::vector<uint32_t>(m_ScreenSize.value.x * m_ScreenSize.value.y, 0xffff00ff);
	std::vector<GEngine::Vector4> m_Accumulation = std::vector<GEngine::Vector4>(m_ScreenSize.value.x * m_ScreenSize.value.y, GEngine::Vector4());
	std::vector<GEngine::Vector3> m_RayDir = std::vector<GEngine::Vector3>(m_ScreenSize.value.x * m_ScreenSize.value.y, { 0, 0, 1 });
	std::vector<Sphere> m_Spheres;
	GEngine::Ref<GEngine::Texture2D> m_Texture;
	float m_AspectRatio = m_ScreenSize.value.x / m_ScreenSize.value.y;
	GEngine::Vector3 m_LightDir = { 1, 1, 0 };
	float m_Bounce = 3;
	std::uniform_real_distribution<float> m_Distribution;
	std::default_random_engine m_Engine;
	uint32_t m_FrameCount = 1;
	GEngine::Vector3 m_CameraPosition = { 0, 0, 0 };
};


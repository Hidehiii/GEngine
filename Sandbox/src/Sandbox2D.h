#pragma once

#include <GEngine.h>

#include "ParticleSystem.h"

class Sandbox2D : public GEngine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate() override;
	virtual void OnGuiRender() override;
	void OnEvent(GEngine::Event& e) override;
private:
	GEngine::Ref<GEngine::Texture2D> m_Texture_01;
	GEngine::Ref<GEngine::Texture2D> m_Texture_02;
	GEngine::Ref<GEngine::Texture2D> m_SpriteSheet;
	GEngine::Ref<GEngine::SubTexture2D> m_TrafficLights, m_Forest;
	GEngine::Ref<GEngine::OrthoGraphicCameraController> m_CameraController;
	GEngine::Vector4 m_Color = {1.0f, 1.0f, 1.0f, 1.0f};
	float m_QuadCount = 0;
	ParticleProps m_Particle;
	ParticleSystem m_ParticleSystem;
	std::unordered_map<char, GEngine::Ref<GEngine::SubTexture2D>> m_TextureMap;
	float m_MapWidth, m_MapHeight;
};


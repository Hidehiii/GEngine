#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Components/Camera/OrthoGraphicCamera.h"
#include "GEngine/Components/Camera/EditorCamera.h"
#include "GEngine/Renderer/Texture.h"
#include "GEngine/Renderer/Shader.h"
#include "GEngine/Renderer/SubTexture2D.h"
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	class GENGINE_API Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();
		static void Flush();

		static void BeginScene();
		static void EndScene();

		static void RenderImage(Transform& transform, ImageRenderer& imagerRenderer);
		static void RenderCircle(Transform& transform, CircleRenderer& circleRenderer);

		// Base
		static void DrawQuad(Transform& transform, const Vector4& color);
		static void DrawQuad(Transform& transform, const Vector4& color, const Ref<Texture2D> tex, const Vector2 tiling = { 1.0f, 1.0f});
		static void DrawQuad(Transform& transform, const Vector4& color, const Ref<SubTexture2D> tex, const Vector2 tiling = { 1.0f, 1.0f});
		static void DrawCircle(Transform& transform, const Vector4& color, const float radius = 0.5f, const float thickness = 1.0f, const float fade = 0.0f);
		static void DrawCircle(Transform& transform, const Vector4& color, const Ref<Texture2D> tex, const float radius = 0.5f, const float thickness = 1.0f, const float fade = 0.0f);
		static void DrawCircle(Transform& transform, const float& radius, const Vector4& lineColor, const int& segmentCount = 36);
		static void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);
		static void DrawLine(const Vector3& start, const Vector3& direction, const float length, const Vector4& color);
		static void DrawRect(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color);
		static void DrawRect(Transform& transform, const Vector4& color);
		static void DrawRect(Matrix4x4& transform, const Vector4& color);
		static void DrawPoint(const Vector3& position, const Vector4& color);
		// Without Texture
		static void DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color);
		static void DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color);

		// With Texture
		static void DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const Vector2 tiling = { 1.0f, 1.0f});
		static void DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const Vector2 tiling = { 1.0f, 1.0f});

		// With SubTexture
		static void DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const Vector2 tiling = { 1.0f, 1.0f});
		static void DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const Vector2 tiling = { 1.0f, 1.0f});

		// With SpriteSheet (unuseful)
		static void DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const Vector2 tiling = { 1.0f, 1.0f});
		static void DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const Vector2 tiling = { 1.0f, 1.0f});

		// Other
		static void SetLineWidth(float width);
		static void SetPointSize(float size);

		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};
		static Statistics GetStats();
		static void ResetStats();
	private:
		static void ResetShaderData();
		static void StartBatch();
		static void NextBatch();
	};
}



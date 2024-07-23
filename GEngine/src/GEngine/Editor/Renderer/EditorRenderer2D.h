#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Components/Camera/OrthoGraphicCamera.h"
#include "GEngine/Components/Camera/EditorCamera.h"
#include "GEngine/Renderer/Texture.h"
#include "GEngine/Renderer/SubTexture2D.h"
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Components/Components.h"
#include "GEngine/Math/Math.h"



namespace GEngine::Editor
{
	class GENGINE_API EditorRenderer2D
	{
	public:
		static void Init();
		static void Shutdown();
		static void Flush();

		static void BeginScene(const OrthoGraphicCamera& camera);
		static void BeginScene(const EditorCamera& camera);
		static void BeginScene(Camera& camera, Transform& transform);
		static void EndScene();

		// Base
		static void DrawQuad(Transform& transform, const Vector4& color, const int GameObjctID);
		static void DrawQuad(Transform& transform, const Vector4& color, const Ref<Texture2D> tex, const int GameObjctID, const Vector2 tiling = { 1.0f, 1.0f });
		static void DrawQuad(Transform& transform, const Vector4& color, const Ref<SubTexture2D> tex, const int GameObjctID, const Vector2 tiling = { 1.0f, 1.0f });

		// Without Texture
		static void DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const int GameObjctID);
		static void DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const int GameObjctID);

		// With Texture
		static void DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const int GameObjctID, const Vector2 tiling = { 1.0f, 1.0f });
		static void DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const int GameObjctID, const Vector2 tiling = { 1.0f, 1.0f });

		// With SubTexture
		static void DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const int GameObjctID, const Vector2 tiling = { 1.0f, 1.0f });
		static void DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const int GameObjctID, const Vector2 tiling = { 1.0f, 1.0f });

		// With SpriteSheet (unuseful)
		static void DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const int GameObjctID, const Vector2 tiling = { 1.0f, 1.0f });
		static void DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const int GameObjctID, const Vector2 tiling = { 1.0f, 1.0f });



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
		static void ResetShaderData(Matrix4x4& vp);
		static void StartBatch();
		static void NextBatch();
	};
}



#include "GEpch.h"
#include "EditorRenderer2D.h"
#include "GEngine/Renderer/Shader.h"
#include "GEngine/Renderer/VertexArray.h"
#include "GEngine/Renderer/Texture.h"
#include "GEngine/Renderer/RenderCommand.h"
#include "GEngine/Components/Transform/Transform.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Core/Time.h"

// Editor only

namespace GEngine::Editor
{
	struct QuadVertex
	{
		Vector4 Position;
		Vector2 UV;
		Vector2 Tiling;
		Vector4 Color;
		int TexIndex;

		int GameObjectID = -1;
	};

	struct EditorRenderer2DData
	{
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture

		Vector4 VertexPositions[4];
		Vector2 VertexUVs[4];

		EditorRenderer2D::Statistics stats;
	};


	static EditorRenderer2DData s_Data;

	void EditorRenderer2D::Init()
	{
		GE_PROFILE_FUNCTION();
		// Runtime
		{
			// Vertex Array
			s_Data.QuadVertexArray = (VertexArray::Create());


			s_Data.QuadVertexBuffer = (VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex)));

			// Vertex Buffer Object
			s_Data.QuadVertexBuffer->SetLayout({
				{ShaderDataType::float4, "PositionOS"},
				{ShaderDataType::float2, "UV"},
				{ShaderDataType::float2, "Tiling"},
				{ShaderDataType::float4, "Color"},
				{ShaderDataType::int1, "TexIndex"},
				{ShaderDataType::int1, "GameObjectID"}
				});



			s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

			uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

			s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];

			uint32_t offset = 0;
			for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
			{
				quadIndices[i + 0] = offset + 0;
				quadIndices[i + 1] = offset + 1;
				quadIndices[i + 2] = offset + 2;

				quadIndices[i + 3] = offset + 2;
				quadIndices[i + 4] = offset + 3;
				quadIndices[i + 5] = offset + 0;

				offset += 4;
			}

			// Index Buffer Object
			Ref<IndexBuffer> m_IndexBuffer;
			m_IndexBuffer = (IndexBuffer::Create(quadIndices, s_Data.MaxIndices));

			s_Data.QuadVertexArray->SetIndexBuffer(m_IndexBuffer);

			delete[] quadIndices;



			s_Data.TextureShader = Shader::Create("Assets/Shaders/QuadShader.glsl");

			s_Data.WhiteTexture = Texture2D::Create(1, 1);
			uint32_t whiteTextureData = 0xffffffff;
			s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));
			s_Data.TextureSlots[0] = s_Data.WhiteTexture;



			s_Data.VertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.VertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
			s_Data.VertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
			s_Data.VertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

			s_Data.VertexUVs[0] = { 0.0f, 0.0f };
			s_Data.VertexUVs[1] = { 1.0f, 0.0f };
			s_Data.VertexUVs[2] = { 1.0f, 1.0f };
			s_Data.VertexUVs[3] = { 0.0f, 1.0f };
		}

	}
	void EditorRenderer2D::Shutdown()
	{

	}
	void EditorRenderer2D::Flush()
	{
		GE_PROFILE_FUNCTION();

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		{
			s_Data.TextureSlots[i]->Bind(i);
		}
		RenderCommand::DrawTriangles(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
		s_Data.stats.DrawCalls++;
	}
	void EditorRenderer2D::BeginScene(const OrthoGraphicCamera& camera)
	{
		GE_PROFILE_FUNCTION();

		ResetShaderData(camera.GetViewProjection());
	}
	void EditorRenderer2D::BeginScene(const EditorCamera& camera)
	{
		GE_PROFILE_FUNCTION();

		Matrix4x4 vp = camera.GetViewProjection();

		ResetShaderData(vp);
	}
	void EditorRenderer2D::BeginScene(Camera& camera, Transform& transform)
	{
		GE_PROFILE_FUNCTION();

		Matrix4x4 vp = camera.GetProjectionMatrix() * Math::Inverse(transform.GetModelMatrix());

		ResetShaderData(vp);
	}
	void EditorRenderer2D::EndScene()
	{
		GE_PROFILE_FUNCTION();

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
		if (dataSize == 0)
		{
			return;
		}
		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void EditorRenderer2D::DrawQuad(Transform& transform, const Vector4& color, const int GameObjctID)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= EditorRenderer2DData::MaxIndices)
		{
			NextBatch();
		}

		const int texIndex = 0.0f; // 0 = white texture

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.VertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->UV = s_Data.VertexUVs[i];
			s_Data.QuadVertexBufferPtr->Tiling = { 1.0f, 1.0f };
			s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
			s_Data.QuadVertexBufferPtr->GameObjectID = GameObjctID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.stats.QuadCount++;
	}
	void EditorRenderer2D::DrawQuad(Transform& transform, const Vector4& color, const Ref<Texture2D> tex, const int GameObjctID, const Vector2 tiling)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= EditorRenderer2DData::MaxIndices)
		{
			NextBatch();
		}

		int textureIndex = 0;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *tex.get())
			{
				textureIndex = i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = tex;
			s_Data.TextureSlotIndex++;
		}


		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.VertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->UV = s_Data.VertexUVs[i];
			s_Data.QuadVertexBufferPtr->Tiling = tiling;
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->GameObjectID = GameObjctID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.stats.QuadCount++;
	}
	void EditorRenderer2D::DrawQuad(Transform& transform, const Vector4& color, const Ref<SubTexture2D> tex, const int GameObjctID, const Vector2 tiling)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= EditorRenderer2DData::MaxIndices)
		{
			NextBatch();
		}

		int textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *(tex->GetTexture()).get())
			{
				textureIndex = i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = tex->GetTexture();
			s_Data.TextureSlotIndex++;
		}



		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.VertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->UV = (tex->GetUVs())[i];
			s_Data.QuadVertexBufferPtr->Tiling = tiling;
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->GameObjectID = GameObjctID;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.stats.QuadCount++;
	}
	void EditorRenderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const int GameObjctID)
	{
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, color, GameObjctID);
	}
	void EditorRenderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const int GameObjctID)
	{
		Transform transform = Transform(position, rotation, size);
		DrawQuad(transform, color, GameObjctID);
	}
	void EditorRenderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const int GameObjctID, const Vector2 tiling)
	{
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, color, tex, GameObjctID, tiling);
	}
	void EditorRenderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const int GameObjctID, const Vector2 tiling)
	{
		Transform transform = Transform(position, rotation, size);

		DrawQuad(transform, color, tex, GameObjctID, tiling);
	}
	void EditorRenderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const int GameObjctID, const Vector2 tiling)
	{
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, color, tex, GameObjctID, tiling);
	}
	void EditorRenderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const int GameObjctID, const Vector2 tiling)
	{

		Transform transform = Transform(position, rotation, size);
		DrawQuad(transform, color, tex, GameObjctID, tiling);

	}
	void EditorRenderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const int GameObjctID, const Vector2 tiling)
	{
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, color, spriteSheet, spriteSheetSize, spriteSize, offset, GameObjctID, tiling);
	}
	void EditorRenderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const int GameObjctID, const Vector2 tiling)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= EditorRenderer2DData::MaxIndices)
		{
			NextBatch();
		}

		int textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *spriteSheet.get())
			{
				textureIndex = i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = spriteSheet;
			s_Data.TextureSlotIndex++;
		}

		// New UV
		Vector2 uv[4] =
		{
			{ (offset.x * spriteSize.x) / spriteSheetSize.x, (offset.y * spriteSize.y) / spriteSheetSize.y},
			{ ((offset.x + 1) * spriteSize.x) / spriteSheetSize.x, (offset.y * spriteSize.y) / spriteSheetSize.y},
			{ ((offset.x + 1) * spriteSize.x) / spriteSheetSize.x, ((offset.y + 1) * spriteSize.y) / spriteSheetSize.y},
			{ (offset.x * spriteSize.x) / spriteSheetSize.x, ((offset.y + 1) * spriteSize.y) / spriteSheetSize.y}
		};

		Transform transform = Transform(position, rotation, size);

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.VertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->UV = uv[i];
			s_Data.QuadVertexBufferPtr->Tiling = tiling;
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->GameObjectID = GameObjctID;
			s_Data.QuadVertexBufferPtr++;
		}


		s_Data.QuadIndexCount += 6;

		s_Data.stats.QuadCount++;
	}

	EditorRenderer2D::Statistics EditorRenderer2D::GetStats()
	{
		return s_Data.stats;
	}
	void EditorRenderer2D::ResetStats()
	{
		memset(&s_Data.stats, 0, sizeof(Statistics));
	}
	void EditorRenderer2D::ResetShaderData(Matrix4x4& vp)
	{
		s_Data.TextureShader->Bind();
		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;


		s_Data.TextureShader->SetIntArray("_Textures", samplers, s_Data.MaxTextureSlots);
		s_Data.TextureShader->SetMat4x4("GE_MATRIX_VP", vp);
		s_Data.TextureShader->SetFloat1("_Time", Time::GetRunTime());
		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.QuadIndexCount = 0;
		s_Data.TextureSlotIndex = 1;
	}
	void EditorRenderer2D::StartBatch()
	{
		EndScene();

		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.QuadIndexCount = 0;
		s_Data.TextureSlotIndex = 1;
	}
	void EditorRenderer2D::NextBatch()
	{
		StartBatch();
	}


}
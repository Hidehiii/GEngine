#include "GEpch.h"
#include "Renderer2D.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"
#include "RenderCommand.h"
#include "GEngine/Components/Transform/Transform.h"
#include "GEngine/Renderer/UniformBuffer.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Core/Time.h"

namespace GEngine
{
	struct QuadVertex
	{
		Vector4 Position;
		Vector2 UV;
		Vector2 Tiling;
		Vector4 Color;
		int TexIndex;
	};

	struct CircleVertex
	{
		Vector4 Position;
		Vector4 Color;
		Vector2 UV;
		float Radius;
		float Thickness;
		int TexIndex;
		float Fade;
	};

	struct LineVertex
	{
		Vector4 Position;
		Vector4 Color;
	};

	struct PointVertex
	{
		Vector4 Position;
		Vector4 Color;
	};

	

	struct Renderer2DData
	{
		static const uint32_t MaxtTiangle		= 20000;
		static const uint32_t MaxVertices		= MaxtTiangle * 3;
		static const uint32_t MaxIndices		= MaxtTiangle * 3;
		static const uint32_t MaxTextureSlots	= 32;

		Ref<VertexArray>	QuadVertexArray;
		Ref<VertexBuffer>	QuadVertexBuffer;
		Ref<Shader>			QuadShader;
		uint32_t			QuadIndexCount			= 0;
		QuadVertex*			QuadVertexBufferBase	= nullptr;
		QuadVertex*			QuadVertexBufferPtr		= nullptr;

		Ref<VertexArray>	CircleVertexArray;
		Ref<VertexBuffer>	CircleVertexBuffer;
		Ref<Shader>			CircleShader;
		uint32_t			CircleIndexCount = 0;
		CircleVertex*		CircleVertexBufferBase	= nullptr;
		CircleVertex*		CircleVertexBufferPtr	= nullptr;

		Ref<VertexArray>	LineVertexArray;
		Ref<VertexBuffer>	LineVertexBuffer;
		Ref<Shader>			LineShader;
		uint32_t			LineVertexCount = 0;
		LineVertex*			LineVertexBufferBase	= nullptr;
		LineVertex*			LineVertexBufferPtr		= nullptr;

		Ref<VertexArray>	PointVertexArray;
		Ref<VertexBuffer>	PointVertexBuffer;
		Ref<Shader>			PointShader;
		uint32_t			PointVertexCount = 0;
		PointVertex*		PointVertexBufferBase	= nullptr;
		PointVertex*		PointVertexBufferPtr	= nullptr;

		std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex					= 1; // 0 = white texture

		Vector4 QuadVertexPositions[4];
		Vector2 QuadVertexUVs[4];

		Ref<Texture2D> WhiteTexture;

		Renderer2D::Statistics stats;

	};

	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		GE_PROFILE_FUNCTION();
		// Runtime
		{
			// Quad
			{
				// Vertex Array
				s_Data.QuadVertexArray		= (VertexArray::Create());

				s_Data.QuadShader = Shader::Create("Assets/Shaders/RayTrackSphere.glsl");
				s_Data.QuadVertexBuffer		= (VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex)));

				// Vertex Buffer Object
				s_Data.QuadVertexBuffer->SetLayout({
					{ShaderDataType::float4,	"PositionOS"},
					{ShaderDataType::float2,	"UV"},
					{ShaderDataType::float2,	"Tiling"},
					{ShaderDataType::float4,	"Color"},
					{ShaderDataType::int1,		"TexIndex"}
					});



				s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

				uint32_t* quadIndices		= new uint32_t[s_Data.MaxIndices];

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
				Ref<IndexBuffer> indexBuffer;
				indexBuffer = (IndexBuffer::Create(quadIndices, s_Data.MaxIndices));

				s_Data.QuadVertexArray->SetIndexBuffer(indexBuffer);

				delete[] quadIndices;



				s_Data.QuadShader = Shader::Create("Assets/Shaders/2D/Quad2D.glsl");
				//s_Data.QuadShader = Shader::Create("Assets/Shaders/RayTrackSphere.glsl");


				s_Data.WhiteTexture = Texture2D::Create(1, 1);
				uint32_t whiteTextureData = 0xffffffff;
				s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(whiteTextureData));
				s_Data.TextureSlots[0] = s_Data.WhiteTexture;



				s_Data.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
				s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f };
				s_Data.QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f };
				s_Data.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

				s_Data.QuadVertexUVs[0] = { 0.0f, 0.0f };
				s_Data.QuadVertexUVs[1] = { 1.0f, 0.0f };
				s_Data.QuadVertexUVs[2] = { 1.0f, 1.0f };
				s_Data.QuadVertexUVs[3] = { 0.0f, 1.0f };
			}
			// Circle
			{
				// Vertex Array
				s_Data.CircleVertexArray = (VertexArray::Create());


				s_Data.CircleVertexBuffer = (VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex)));

				// Vertex Buffer Object
				s_Data.CircleVertexBuffer->SetLayout({
					{ShaderDataType::float4, "PositionOS"},
					{ShaderDataType::float4, "Color"},
					{ShaderDataType::float2, "UV"},
					{ShaderDataType::float1, "Radius" },
					{ShaderDataType::float1, "Thickness"},
					{ShaderDataType::int1, "TexIndex"},
					{ShaderDataType::float1, "Fade"}
					});



				s_Data.CircleVertexArray->AddVertexBuffer(s_Data.CircleVertexBuffer);

				uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

				s_Data.CircleVertexBufferBase = new CircleVertex[s_Data.MaxVertices];

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
				Ref<IndexBuffer> indexBuffer;
				indexBuffer = (IndexBuffer::Create(quadIndices, s_Data.MaxIndices));

				s_Data.CircleVertexArray->SetIndexBuffer(indexBuffer);

				delete[] quadIndices;



				s_Data.CircleShader = Shader::Create("Assets/Shaders/2D/Circle2D.glsl");
			}

			// Line
			{
				// Vertex Array
				s_Data.LineVertexArray = (VertexArray::Create());


				s_Data.LineVertexBuffer = (VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex)));

				// Vertex Buffer Object
				s_Data.LineVertexBuffer->SetLayout({
					{ShaderDataType::float4, "PositionOS"},
					{ShaderDataType::float4, "Color"}
					});

				s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];

				s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);

				s_Data.LineShader = Shader::Create("Assets/Shaders/2D/Line2D.glsl");
			}

			// Point
			{
				// Vertex Array
				s_Data.PointVertexArray = (VertexArray::Create());


				s_Data.PointVertexBuffer = (VertexBuffer::Create(s_Data.MaxVertices * sizeof(PointVertex)));

				// Vertex Buffer Object
				s_Data.PointVertexBuffer->SetLayout({
					{ShaderDataType::float4, "PositionOS"},
					{ShaderDataType::float4, "Color"}
					});

				s_Data.PointVertexBufferBase = new PointVertex[s_Data.MaxVertices];

				s_Data.PointVertexArray->AddVertexBuffer(s_Data.PointVertexBuffer);

				s_Data.PointShader = Shader::Create("Assets/Shaders/2D/Point2D.glsl");
			}
		}
		
	}
	void Renderer2D::Shutdown()
	{

	}
	void Renderer2D::Flush()
	{
		GE_PROFILE_FUNCTION();

		

		if (s_Data.QuadIndexCount)
		{
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			{
				s_Data.TextureSlots[i]->Bind(i);
			}
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
			s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);
			s_Data.QuadShader->Bind();
			RenderCommand::DrawTriangles(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
			s_Data.stats.m_DrawCalls++;
		}

		if (s_Data.CircleIndexCount)
		{
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			{
				s_Data.TextureSlots[i]->Bind(i);
			}
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
			s_Data.CircleVertexBuffer->SetData(s_Data.CircleVertexBufferBase, dataSize);
			s_Data.CircleShader->Bind();
			RenderCommand::DrawTriangles(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
			s_Data.stats.m_DrawCalls++;
		}

		if (s_Data.LineVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
			s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase, dataSize);
			s_Data.LineShader->Bind();
			RenderCommand::DrawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
			s_Data.stats.m_DrawCalls++;
		}

		if (s_Data.PointVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.PointVertexBufferPtr - (uint8_t*)s_Data.PointVertexBufferBase);
			s_Data.PointVertexBuffer->SetData(s_Data.PointVertexBufferBase, dataSize);
			s_Data.PointShader->Bind();
			RenderCommand::DrawPoints(s_Data.PointVertexArray, s_Data.PointVertexCount);
			s_Data.stats.m_DrawCalls++;
		}
	}
	void Renderer2D::BeginScene()
	{
		GE_PROFILE_FUNCTION();

		ResetShaderData();
	}
	void Renderer2D::EndScene()
	{
		GE_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer2D::DrawQuad(Transform& transform, const Vector4& color)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{
			NextBatch();
		}

		const int texIndex = 0.0f; // 0 = white texture

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->UV = s_Data.QuadVertexUVs[i];
			s_Data.QuadVertexBufferPtr->Tiling = { 1.0f, 1.0f };
			s_Data.QuadVertexBufferPtr->TexIndex = texIndex;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.stats.m_TriangleCount += 2;
	}
	void Renderer2D::DrawQuad(Transform& transform, const Vector4& color, const Ref<Texture2D> tex, const Vector2 tiling)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
		{
			NextBatch();
		}

		int textureIndex = 0.0f;
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
			s_Data.QuadVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->UV = s_Data.QuadVertexUVs[i];
			s_Data.QuadVertexBufferPtr->Tiling = tiling;
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.stats.m_TriangleCount += 2;
	}
	void Renderer2D::DrawQuad(Transform& transform, const Vector4& color, const Ref<SubTexture2D> tex, const Vector2 tiling)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
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
			s_Data.QuadVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->UV = (tex->GetUVs())[i];
			s_Data.QuadVertexBufferPtr->Tiling = tiling;
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.stats.m_TriangleCount += 2;
	}
	void Renderer2D::DrawCircle(Transform& transform, const Vector4& color, const float radius, const float thickness, const float fade)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.CircleIndexCount >= Renderer2DData::MaxIndices)
		{
			NextBatch();
		}

		const int texIndex = 0.0f; // 0 = white texture

		for (int i = 0; i < 4; i++)
		{
			s_Data.CircleVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.QuadVertexPositions[i];
			s_Data.CircleVertexBufferPtr->Color = color;
			s_Data.CircleVertexBufferPtr->UV = s_Data.QuadVertexUVs[i];
			s_Data.CircleVertexBufferPtr->Radius = radius;
			s_Data.CircleVertexBufferPtr->Thickness = thickness;
			s_Data.CircleVertexBufferPtr->TexIndex = texIndex;
			s_Data.CircleVertexBufferPtr->Fade = fade;
			s_Data.CircleVertexBufferPtr++;
		}

		s_Data.CircleIndexCount += 6;

		s_Data.stats.m_TriangleCount += 2;
	}
	void Renderer2D::DrawCircle(Transform& transform, const Vector4& color, const Ref<Texture2D> tex, const float radius, const float thickness, const float fade)
	{
		GE_PROFILE_FUNCTION();
		if (s_Data.CircleIndexCount >= Renderer2DData::MaxIndices)
		{
			NextBatch();
		}

		int textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *(tex).get())
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


		if (s_Data.CircleIndexCount >= Renderer2DData::MaxIndices)
		{
			NextBatch();
		}

		for (int i = 0; i < 4; i++)
		{
			s_Data.CircleVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.QuadVertexPositions[i];
			s_Data.CircleVertexBufferPtr->Color = color;
			s_Data.CircleVertexBufferPtr->Radius = radius;
			s_Data.CircleVertexBufferPtr->Thickness = thickness;
			s_Data.CircleVertexBufferPtr->TexIndex = textureIndex;
			s_Data.CircleVertexBufferPtr->Fade = fade;
			s_Data.CircleVertexBufferPtr++;
		}

		s_Data.CircleIndexCount += 6;

		s_Data.stats.m_TriangleCount += 2;
	}
	void Renderer2D::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.LineVertexCount >= Renderer2DData::MaxVertices)
		{
			NextBatch();
		}

		s_Data.LineVertexBufferPtr->Position = { start.value.x, start.value.y, start.value.z, 1.0f };
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = { end.value.x, end.value.y, end.value.z, 1.0f };
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexCount += 2;

		s_Data.stats.m_LineCount++;
	}
	void Renderer2D::DrawCircle(Transform& transform, const float& radius, const Vector4& lineColor, const int& segmentCount)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.LineVertexCount >= Renderer2DData::MaxVertices)
		{
			NextBatch();
		}

		for (int i = 0; i < segmentCount; i++)
		{
			float angle = (float)i / (float)segmentCount * Math::PI * 2.0f;
			float nextAngle = (float)(i + 1) / (float)segmentCount * Math::PI * 2.0f;

			Vector4 start = { Math::Cos(angle) * radius, Math::Sin(angle) * radius, 0.0f, 1.0f };
			Vector4 end = { Math::Cos(nextAngle) * radius, Math::Sin(nextAngle) * radius, 0.0f, 1.0f };

			DrawLine(transform.GetModelMatrix() * start, transform.GetModelMatrix() * end, lineColor);
		}
	}
	void Renderer2D::DrawLine(const Vector3& start, const Vector3& direction, const float length, const Vector4& color)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.LineVertexCount >= Renderer2DData::MaxVertices)
		{
			NextBatch();
		}

		Vector4 startPos = { start.value.x, start.value.y, start.value.z, 1.0f };

		s_Data.LineVertexBufferPtr->Position = startPos;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = startPos + Vector4(direction.value.x, direction.value.y, direction.value.z, 0.0f) * length;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexCount += 2;

		s_Data.stats.m_LineCount++;
	}
	void Renderer2D::DrawPoint(const Vector3& position, const Vector4& color)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.PointVertexCount >= Renderer2DData::MaxVertices)
		{
			NextBatch();
		}

		s_Data.PointVertexBufferPtr->Position = { position.value.x, position.value.y, position.value.z, 1.0f };
		s_Data.PointVertexBufferPtr->Color = color;
		s_Data.PointVertexBufferPtr++;

		s_Data.PointVertexCount++;

		s_Data.stats.m_PointCount++;
	}
	void Renderer2D::DrawRect(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color)
	{
		Transform transform = Transform(position, rotation, size);
		DrawRect(transform, color);
	}
	void Renderer2D::DrawRect(Transform& transform, const Vector4& color)
	{
		DrawRect(transform.GetModelMatrix(), color);
	}
	void Renderer2D::DrawRect(Matrix4x4& transform, const Vector4& color)
	{
		Vector4 pos[4];
		for (int i = 0; i < 4; i++)
		{
			pos[i] = transform * s_Data.QuadVertexPositions[i];
		}
		DrawLine(pos[0], pos[1], color);
		DrawLine(pos[1], pos[2], color);
		DrawLine(pos[2], pos[3], color);
		DrawLine(pos[3], pos[0], color);
	}
	void Renderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color)
	{
		DrawQuad({ position.value.x, position.value.y, 0.0f }, rotation, size, color);
	}
	void Renderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color)
	{
		Transform transform = Transform(position, rotation, size);
		DrawQuad(transform, color);
	}
	void Renderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const Vector2 tiling)
	{
		DrawQuad({ position.value.x, position.value.y, 0.0f }, rotation, size, color, tex, tiling);
	}
	void Renderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const Vector2 tiling)
	{
		Transform transform = Transform(position, rotation, size);

		DrawQuad(transform, color, tex, tiling);
	}
	void Renderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const Vector2 tiling)
	{
		DrawQuad({ position.value.x, position.value.y, 0.0f }, rotation, size, color, tex, tiling);
	}
	void Renderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const Vector2 tiling)
	{

		Transform transform = Transform(position, rotation, size);
		DrawQuad(transform, color, tex, tiling);
		
	}
	void Renderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const Vector2 tiling)
	{
		DrawQuad({ position.value.x, position.value.y, 0.0f }, rotation, size, color, spriteSheet, spriteSheetSize, spriteSize, offset, tiling);
	}
	void Renderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const Vector2 tiling)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
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
			{ (offset.value.x * spriteSize.value.x) / spriteSheetSize.value.x, (offset.value.y * spriteSize.value.y) / spriteSheetSize.value.y},
			{ ((offset.value.x + 1) * spriteSize.value.x) / spriteSheetSize.value.x, (offset.value.y * spriteSize.value.y) / spriteSheetSize.value.y},
			{ ((offset.value.x + 1) * spriteSize.value.x) / spriteSheetSize.value.x, ((offset.value.y + 1) * spriteSize.value.y) / spriteSheetSize.value.y},
			{ (offset.value.x * spriteSize.value.x) / spriteSheetSize.value.x, ((offset.value.y + 1) * spriteSize.value.y) / spriteSheetSize.value.y}
		};

		Transform transform = Transform(position, rotation, size);

		for (int i = 0; i < 4; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform.GetModelMatrix() * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->UV = uv[i];
			s_Data.QuadVertexBufferPtr->Tiling = tiling;
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr++;
		}
		

		s_Data.QuadIndexCount += 6;

		s_Data.stats.m_TriangleCount += 2;
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.stats;
	}
	void Renderer2D::ResetStats()
	{
		memset(&s_Data.stats, 0, sizeof(Statistics));
	}
	void Renderer2D::ResetShaderData()
	{
		
		int32_t samplers[s_Data.MaxTextureSlots];
		for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
			samplers[i] = i;

		s_Data.QuadShader->Bind();
		s_Data.QuadShader->SetIntArray("_Textures", samplers, s_Data.MaxTextureSlots);

		s_Data.CircleShader->Bind();
		s_Data.CircleShader->SetIntArray("_Textures", samplers, s_Data.MaxTextureSlots);


		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.QuadIndexCount = 0;
		
		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;
		s_Data.CircleIndexCount = 0;

		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
		s_Data.LineVertexCount = 0;

		s_Data.PointVertexBufferPtr = s_Data.PointVertexBufferBase;
		s_Data.PointVertexCount = 0;
	}
	void Renderer2D::StartBatch()
	{
		Flush();
		s_Data.TextureSlotIndex = 1;

		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
		s_Data.QuadIndexCount = 0;

		s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;
		s_Data.CircleIndexCount = 0;

		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
		s_Data.LineVertexCount = 0;

		s_Data.PointVertexBufferPtr = s_Data.PointVertexBufferBase;
		s_Data.PointVertexCount = 0;
	}
	void Renderer2D::NextBatch()
	{
		StartBatch();
	}
	void Renderer2D::SetLineWidth(float width)
	{
		RenderCommand::SetLineWidth(width);
	}
	void Renderer2D::SetPointSize(float size)
	{
		RenderCommand::SetPointSize(size);
	}
	void Renderer2D::RenderImage(Transform& transform, ImageRenderer& imagerRenderer)
	{
		if (imagerRenderer.m_Texture)
		{
			DrawQuad(transform, imagerRenderer.m_Color, imagerRenderer.m_Texture, imagerRenderer.m_Tiling);
		}
		else
		{
			DrawQuad(transform, imagerRenderer.m_Color);
		}
	}
	void Renderer2D::RenderCircle(Transform& transform, CircleRenderer& circleRenderer)
	{
		if (circleRenderer.m_Texture)
		{
			DrawCircle(transform, circleRenderer.m_Color, circleRenderer.m_Texture, circleRenderer.m_Radius, circleRenderer.m_Thickness, circleRenderer.m_Fade);
		}
		else
		{
			DrawCircle(transform, circleRenderer.m_Color, circleRenderer.m_Radius, circleRenderer.m_Thickness, circleRenderer.m_Fade);
		}
	}
}

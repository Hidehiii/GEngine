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
#include "GEngine/Renderer/Pipeline.h"

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

		Ref<Pipeline>		QuadPipeline;
		uint32_t			QuadIndexCount			= 0;
		uint32_t			QuadTextureIndex		= 1; // 0 = white texture
		QuadVertex*			QuadVertexBufferBase	= nullptr;
		QuadVertex*			QuadVertexBufferPtr		= nullptr;

		Ref<Pipeline>		CirclePipeline;
		uint32_t			CircleIndexCount		= 0;
		uint32_t			CircleTextureIndex		= 1; // 0 = white texture
		CircleVertex*		CircleVertexBufferBase	= nullptr;
		CircleVertex*		CircleVertexBufferPtr	= nullptr;

		Ref<Pipeline>		LinePipeline;
		uint32_t			LineVertexCount = 0;
		LineVertex*			LineVertexBufferBase	= nullptr;
		LineVertex*			LineVertexBufferPtr		= nullptr;

		Ref<Pipeline>		PointPipeline;
		uint32_t			PointVertexCount = 0;
		PointVertex*		PointVertexBufferBase	= nullptr;
		PointVertex*		PointVertexBufferPtr	= nullptr;

		Vector4 QuadVertexPositions[4];
		Vector2 QuadVertexUVs[4];

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
				s_Data.QuadPipeline = Pipeline::Create(
					Material::Create(Shader::Create("Assets/Shaders/2D/Quad2D.glsl")),
					VertexArray::Create(),
					VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex))
				);
				s_Data.QuadPipeline->GetVertexBuffer()->SetLayout({
					{ShaderDataType::float4,	"PositionOS"},
					{ShaderDataType::float2,	"UV"},
					{ShaderDataType::float2,	"Tiling"},
					{ShaderDataType::float4,	"Color"},
					{ShaderDataType::int1,		"TexIndex"}
					});
				s_Data.QuadPipeline->GetVertexArray()->SetVertexBuffer(s_Data.QuadPipeline->GetVertexBuffer());
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
				s_Data.QuadPipeline->GetVertexArray()->SetIndexBuffer(IndexBuffer::Create(quadIndices, s_Data.MaxIndices));
				delete[] quadIndices;


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

				s_Data.CirclePipeline = Pipeline::Create(
					Material::Create(Shader::Create("Assets/Shaders/2D/Circle2D.glsl")),
					VertexArray::Create(),
					VertexBuffer::Create(s_Data.MaxVertices * sizeof(CircleVertex))
				);
				s_Data.CirclePipeline->GetVertexBuffer()->SetLayout({
					{ShaderDataType::float4, "PositionOS"},
					{ShaderDataType::float4, "Color"},
					{ShaderDataType::float2, "UV"},
					{ShaderDataType::float1, "Radius" },
					{ShaderDataType::float1, "Thickness"},
					{ShaderDataType::int1, "TexIndex"},
					{ShaderDataType::float1, "Fade"}
					});
				s_Data.CirclePipeline->GetVertexArray()->SetVertexBuffer(s_Data.CirclePipeline->GetVertexBuffer());

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
				s_Data.CirclePipeline->GetVertexArray()->SetIndexBuffer(IndexBuffer::Create(quadIndices, s_Data.MaxIndices));

				delete[] quadIndices;
			}

			// Line
			{
				s_Data.LinePipeline = Pipeline::Create(
					Material::Create(Shader::Create("Assets/Shaders/2D/Line2D.glsl")),
					VertexArray::Create(),
					VertexBuffer::Create(s_Data.MaxVertices * sizeof(LineVertex))
				);
				s_Data.LinePipeline->GetVertexBuffer()->SetLayout({
					{ShaderDataType::float4, "PositionOS"},
					{ShaderDataType::float4, "Color"}
					});

				s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];
				s_Data.LinePipeline->GetVertexArray()->SetVertexBuffer(s_Data.LinePipeline->GetVertexBuffer());
			}

			// Point
			{
				s_Data.PointPipeline = Pipeline::Create(
					Material::Create(Shader::Create("Assets/Shaders/2D/Point2D.glsl")),
					VertexArray::Create(),
					VertexBuffer::Create(s_Data.MaxVertices * sizeof(PointVertex))
				);
				s_Data.PointPipeline->GetVertexBuffer()->SetLayout({
					{ShaderDataType::float4, "PositionOS"},
					{ShaderDataType::float4, "Color"}
					});
				s_Data.PointVertexBufferBase = new PointVertex[s_Data.MaxVertices];
				s_Data.PointPipeline->GetVertexArray()->SetVertexBuffer(s_Data.PointPipeline->GetVertexBuffer());
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
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase);
			s_Data.QuadPipeline->GetVertexBuffer()->SetData(s_Data.QuadVertexBufferBase, dataSize);
			s_Data.QuadPipeline->Bind();
			RenderCommand::DrawTriangles(s_Data.QuadPipeline->GetVertexArray(), s_Data.QuadIndexCount);
			s_Data.stats.m_DrawCalls++;
		}

		if (s_Data.CircleIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.CircleVertexBufferBase);
			s_Data.CirclePipeline->GetVertexBuffer()->SetData(s_Data.CircleVertexBufferBase, dataSize);
			s_Data.CirclePipeline->Bind();
			RenderCommand::DrawTriangles(s_Data.CirclePipeline->GetVertexArray(), s_Data.CircleIndexCount);
			s_Data.stats.m_DrawCalls++;
		}

		if (s_Data.LineVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
			s_Data.LinePipeline->GetVertexBuffer()->SetData(s_Data.LineVertexBufferBase, dataSize);
			s_Data.LinePipeline->Bind();
			RenderCommand::DrawLines(s_Data.LinePipeline->GetVertexArray(), s_Data.LineVertexCount);
			s_Data.stats.m_DrawCalls++;
		}

		if (s_Data.PointVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.PointVertexBufferPtr - (uint8_t*)s_Data.PointVertexBufferBase);
			s_Data.PointPipeline->GetVertexBuffer()->SetData(s_Data.PointVertexBufferBase, dataSize);
			s_Data.PointPipeline->Bind();
			RenderCommand::DrawPoints(s_Data.PointPipeline->GetVertexArray(), s_Data.PointVertexCount);
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

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices || s_Data.QuadTextureIndex >= s_Data.QuadPipeline->GetMaterial()->GetGetTexture2Ds().size())
		{
			NextBatch();
		}

		int textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.QuadTextureIndex; i++)
		{
			if(*(s_Data.QuadPipeline->GetMaterial()->GetGetTexture2Ds().at(i).Texture.get()) == *tex.get())
			{
				textureIndex = i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = s_Data.QuadTextureIndex;
			s_Data.QuadPipeline->GetMaterial()->GetGetTexture2Ds().at(s_Data.QuadTextureIndex).Texture = tex;
			s_Data.QuadTextureIndex++;
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

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices || s_Data.QuadTextureIndex >= s_Data.QuadPipeline->GetMaterial()->GetGetTexture2Ds().size())
		{
			NextBatch();
		}

		int textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.QuadTextureIndex; i++)
		{
			if (*(s_Data.QuadPipeline->GetMaterial()->GetGetTexture2Ds().at(i).Texture.get()) == *(tex->GetTexture()).get())
			{
				textureIndex = i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = s_Data.QuadTextureIndex;
			s_Data.QuadPipeline->GetMaterial()->GetGetTexture2Ds().at(s_Data.QuadTextureIndex).Texture = tex->GetTexture();
			s_Data.QuadTextureIndex++;
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
		if (s_Data.CircleIndexCount >= Renderer2DData::MaxIndices || s_Data.CircleTextureIndex >= s_Data.CirclePipeline->GetMaterial()->GetGetTexture2Ds().size())
		{
			NextBatch();
		}

		int textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.CircleTextureIndex; i++)
		{
			if (*(s_Data.CirclePipeline->GetMaterial()->GetGetTexture2Ds().at(i).Texture.get()) == *tex.get())
			{
				textureIndex = i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = s_Data.CircleTextureIndex;
			s_Data.CirclePipeline->GetMaterial()->GetGetTexture2Ds().at(s_Data.CircleTextureIndex).Texture = tex;
			s_Data.CircleTextureIndex++;
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

		s_Data.LineVertexBufferPtr->Position = { start.x, start.y, start.z, 1.0f };
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = { end.x, end.y, end.z, 1.0f };
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

		Vector4 startPos = { start.x, start.y, start.z, 1.0f };

		s_Data.LineVertexBufferPtr->Position = startPos;
		s_Data.LineVertexBufferPtr->Color = color;
		s_Data.LineVertexBufferPtr++;

		s_Data.LineVertexBufferPtr->Position = startPos + Vector4(direction.x, direction.y, direction.z, 0.0f) * length;
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

		s_Data.PointVertexBufferPtr->Position = { position.x, position.y, position.z, 1.0f };
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
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, color);
	}
	void Renderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color)
	{
		Transform transform = Transform(position, rotation, size);
		DrawQuad(transform, color);
	}
	void Renderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const Vector2 tiling)
	{
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, color, tex, tiling);
	}
	void Renderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> tex, const Vector2 tiling)
	{
		Transform transform = Transform(position, rotation, size);

		DrawQuad(transform, color, tex, tiling);
	}
	void Renderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const Vector2 tiling)
	{
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, color, tex, tiling);
	}
	void Renderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<SubTexture2D> tex, const Vector2 tiling)
	{

		Transform transform = Transform(position, rotation, size);
		DrawQuad(transform, color, tex, tiling);
		
	}
	void Renderer2D::DrawQuad(const Vector2& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const Vector2 tiling)
	{
		DrawQuad({ position.x, position.y, 0.0f }, rotation, size, color, spriteSheet, spriteSheetSize, spriteSize, offset, tiling);
	}
	void Renderer2D::DrawQuad(const Vector3& position, const Vector3& rotation, const Vector3& size, const Vector4& color, const Ref<Texture2D> spriteSheet, const Vector2 spriteSheetSize, const Vector2 spriteSize, const Vector2 offset, const Vector2 tiling)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices || s_Data.QuadTextureIndex >= s_Data.QuadPipeline->GetMaterial()->GetGetTexture2Ds().size())
		{
			NextBatch();
		}

		int textureIndex = 0.0f;
		for (uint32_t i = 1; i < s_Data.QuadTextureIndex; i++)
		{
			if (*(s_Data.QuadPipeline->GetMaterial()->GetGetTexture2Ds().at(i).Texture.get()) == *spriteSheet.get())
			{
				textureIndex = i;
				break;
			}
		}
		if (textureIndex == 0)
		{
			textureIndex = s_Data.QuadTextureIndex;
			s_Data.QuadPipeline->GetMaterial()->GetGetTexture2Ds().at(s_Data.QuadTextureIndex).Texture = spriteSheet;
			s_Data.QuadTextureIndex++;
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
		s_Data.QuadTextureIndex = 1;
		s_Data.CircleTextureIndex = 1;

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

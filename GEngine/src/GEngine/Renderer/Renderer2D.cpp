#include "GEpch.h"
#include "Renderer2D.h"
#include "Shader.h"

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
		Vector4 PositionOS;
		Vector2 UV;
	};

	struct QuadVertexInstance
	{
		Vector2 Tiling;
		Vector4 Color;
		int		TexIndex;
		Vector4 ModelMatrixRow_1;
		Vector4 ModelMatrixRow_2;
		Vector4 ModelMatrixRow_3;
		Vector4 ModelMatrixRow_4;
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
		static const uint32_t MaxInstance		= 20000;

		Ref<Pipeline>			QuadPipeline;
		uint32_t				QuadInstanceCount			= 0;
		uint32_t				QuadTextureIndex		= 1; // 0 = white texture
		QuadVertexInstance*		QuadInstanceBufferBase	= nullptr;
		QuadVertexInstance*		QuadInstanceBufferPtr	= nullptr;

		Ref<Pipeline>		LinePipeline;
		uint32_t			LineVertexCount = 0;
		LineVertex*			LineVertexBufferBase	= nullptr;
		LineVertex*			LineVertexBufferPtr		= nullptr;

		Ref<Pipeline>		PointPipeline;
		uint32_t			PointVertexCount = 0;
		PointVertex*		PointVertexBufferBase	= nullptr;
		PointVertex*		PointVertexBufferPtr	= nullptr;

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
					Material::Create(Shader::Create("Assets/Shaders/2D/Quad2DInstance.glsl")),
					VertexBuffer::Create(6 * sizeof(QuadVertex), s_Data.MaxInstance * sizeof(QuadVertexInstance))
				);
				s_Data.QuadPipeline->GetVertexBuffer()->SetLayout({
					{ShaderDataType::float4,	"PositionOS"			},
					{ShaderDataType::float2,	"UV"					},
					{ShaderDataType::float2,	"Tiling",			true},
					{ShaderDataType::float4,	"Color",			true},
					{ShaderDataType::int1,		"TexIndex",			true},
					{ShaderDataType::float4,	"ModelMatrixRow_1", true},
					{ShaderDataType::float4,	"ModelMatrixRow_2", true},
					{ShaderDataType::float4,	"ModelMatrixRow_3", true},
					{ShaderDataType::float4,	"ModelMatrixRow_4", true}
					});
				uint32_t* quadIndices = new uint32_t[6];
				s_Data.QuadInstanceBufferBase = new QuadVertexInstance[s_Data.MaxInstance];
				quadIndices[0] = 0;
				quadIndices[1] = 1;
				quadIndices[2] = 2;
				quadIndices[3] = 2;
				quadIndices[4] = 3;
				quadIndices[5] = 0;
				s_Data.QuadPipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(quadIndices, 6));
				delete[] quadIndices;
				QuadVertex* quadVertex = new QuadVertex[4];

				quadVertex[0].PositionOS = { -0.5f, -0.5f, 0.0f, 1.0f };
				quadVertex[1].PositionOS = { 0.5f, -0.5f, 0.0f, 1.0f };
				quadVertex[2].PositionOS = { 0.5f,  0.5f, 0.0f, 1.0f };
				quadVertex[3].PositionOS = { -0.5f,  0.5f, 0.0f, 1.0f };

				quadVertex[0].UV = { 0.0f, 0.0f };
				quadVertex[1].UV = { 1.0f, 0.0f };
				quadVertex[2].UV = { 1.0f, 1.0f };
				quadVertex[3].UV = { 0.0f, 1.0f };

				s_Data.QuadPipeline->GetVertexBuffer()->SetData(quadVertex, 4 * sizeof(QuadVertex));
			}

			// Line
			{
				s_Data.LinePipeline = Pipeline::Create(
					Material::Create(Shader::Create("Assets/Shaders/2D/Line2D.glsl")),
					VertexBuffer::Create(s_Data.MaxInstance * sizeof(LineVertex), 0, VertexTopology::Line)
				);
				s_Data.LinePipeline->GetVertexBuffer()->SetLayout({
					{ShaderDataType::float4, "PositionOS"},
					{ShaderDataType::float4, "Color"}
					});

				s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxInstance];
				uint32_t* lineIndices = new uint32_t[s_Data.MaxInstance];
				uint32_t offset = 0;
				for (uint32_t i = 0; i < s_Data.MaxInstance; i += 2)
				{
					lineIndices[i + 0] = offset + 0;
					lineIndices[i + 1] = offset + 1;

					offset += 2;
				}
				s_Data.LinePipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(lineIndices, s_Data.MaxInstance));
				delete[] lineIndices;

			}

			// Point
			{
				s_Data.PointPipeline = Pipeline::Create(
					Material::Create(Shader::Create("Assets/Shaders/2D/Point2D.glsl")),
					VertexBuffer::Create(s_Data.MaxInstance * sizeof(PointVertex), 0, VertexTopology::Point)
				);
				s_Data.PointPipeline->GetVertexBuffer()->SetLayout({
					{ShaderDataType::float4, "PositionOS"},
					{ShaderDataType::float4, "Color"}
					});
				s_Data.PointVertexBufferBase = new PointVertex[s_Data.MaxInstance];

				uint32_t* pointIndices = new uint32_t[s_Data.MaxInstance];
				uint32_t offset = 0;
				for (uint32_t i = 0; i < s_Data.MaxInstance; i += 1)
				{
					pointIndices[i + 0] = offset + 0;
					offset += 1;
				}
				s_Data.PointPipeline->GetVertexBuffer()->SetIndexBuffer(IndexBuffer::Create(pointIndices, s_Data.MaxInstance));

				delete[] pointIndices;
			}
		}
		
	}
	void Renderer2D::Shutdown()
	{

	}
	void Renderer2D::Flush()
	{
		GE_PROFILE_FUNCTION();

		

		if (s_Data.QuadInstanceCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadInstanceBufferPtr - (uint8_t*)s_Data.QuadInstanceBufferBase);
			s_Data.QuadPipeline->GetVertexBuffer()->SetDataInstance(s_Data.QuadInstanceBufferBase, dataSize);
			s_Data.QuadPipeline->Render( 6, s_Data.QuadInstanceCount);
			s_Data.stats.m_DrawCalls++;
		}

		if (s_Data.LineVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.LineVertexBufferBase);
			s_Data.LinePipeline->GetVertexBuffer()->SetData(s_Data.LineVertexBufferBase, dataSize);
			s_Data.LinePipeline->Render(s_Data.LineVertexCount);
			s_Data.stats.m_DrawCalls++;
		}

		if (s_Data.PointVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.PointVertexBufferPtr - (uint8_t*)s_Data.PointVertexBufferBase);
			s_Data.PointPipeline->GetVertexBuffer()->SetData(s_Data.PointVertexBufferBase, dataSize);
			s_Data.PointPipeline->Render(s_Data.PointVertexCount);
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

		if (s_Data.QuadInstanceCount >= Renderer2DData::MaxInstance)
		{
			NextBatch();
		}

		const int texIndex = 0.0f; // 0 = white texture

		s_Data.QuadInstanceBufferPtr->Tiling				= { 1.0f, 1.0f };
		s_Data.QuadInstanceBufferPtr->Color					= color;
		s_Data.QuadInstanceBufferPtr->TexIndex				= texIndex;
		auto												matrix = transform.GetModelMatrix();
		s_Data.QuadInstanceBufferPtr->ModelMatrixRow_1		= matrix[0];
		s_Data.QuadInstanceBufferPtr->ModelMatrixRow_2		= matrix[1];
		s_Data.QuadInstanceBufferPtr->ModelMatrixRow_3		= matrix[2];
		s_Data.QuadInstanceBufferPtr->ModelMatrixRow_4		= matrix[3];
		s_Data.QuadInstanceBufferPtr++;

		s_Data.QuadInstanceCount += 1;
	}
	void Renderer2D::DrawQuad(Transform& transform, const Vector4& color, const Ref<Texture2D> tex, const Vector2 tiling)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.QuadInstanceCount >= Renderer2DData::MaxInstance || s_Data.QuadTextureIndex >= s_Data.QuadPipeline->GetMaterial()->GetTexture2Ds().size())
		{
			NextBatch();
		}

		int textureIndex = -1;
		for (uint32_t i = 0; i < s_Data.QuadTextureIndex; i++)
		{
			if(*(s_Data.QuadPipeline->GetMaterial()->GetTexture2Ds().at(i).Texture.get()) == *tex.get())
			{
				textureIndex = i;
				break;
			}
		}
		if (textureIndex == -1)
		{
			textureIndex = s_Data.QuadTextureIndex;
			s_Data.QuadPipeline->GetMaterial()->GetTexture2Ds().at(s_Data.QuadTextureIndex).Texture = tex;
			s_Data.QuadTextureIndex++;
		}

		s_Data.QuadInstanceBufferPtr->Tiling			= tiling;
		s_Data.QuadInstanceBufferPtr->Color				= color;
		s_Data.QuadInstanceBufferPtr->TexIndex			= textureIndex;
		auto											matrix = transform.GetModelMatrix();
		s_Data.QuadInstanceBufferPtr->ModelMatrixRow_1	= matrix[0];
		s_Data.QuadInstanceBufferPtr->ModelMatrixRow_2	= matrix[1];
		s_Data.QuadInstanceBufferPtr->ModelMatrixRow_3	= matrix[2];
		s_Data.QuadInstanceBufferPtr->ModelMatrixRow_4	= matrix[3];
		s_Data.QuadInstanceBufferPtr++;

		s_Data.QuadInstanceCount += 1;
	}
	void Renderer2D::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
	{
		GE_PROFILE_FUNCTION();

		if (s_Data.LineVertexCount >= Renderer2DData::MaxInstance)
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

		if (s_Data.LineVertexCount >= Renderer2DData::MaxInstance)
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

		if (s_Data.LineVertexCount >= Renderer2DData::MaxInstance)
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

		if (s_Data.PointVertexCount >= Renderer2DData::MaxInstance)
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
		pos[0] = transform * Vector4{ -0.5f, -0.5f, 0.0f, 1.0f };
		pos[1] = transform * Vector4{  0.5f, -0.5f, 0.0f, 1.0f };
		pos[2] = transform * Vector4{  0.5f,  0.5f, 0.0f, 1.0f };
		pos[3] = transform * Vector4{ -0.5f,  0.5f, 0.0f, 1.0f };
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
		s_Data.QuadInstanceBufferPtr = s_Data.QuadInstanceBufferBase;
		s_Data.QuadInstanceCount = 0;

		s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
		s_Data.LineVertexCount = 0;

		s_Data.PointVertexBufferPtr = s_Data.PointVertexBufferBase;
		s_Data.PointVertexCount = 0;
	}
	void Renderer2D::StartBatch()
	{
		Flush();
		s_Data.QuadTextureIndex = 1;

		s_Data.QuadInstanceBufferPtr = s_Data.QuadInstanceBufferBase;
		s_Data.QuadInstanceCount = 0;

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
}

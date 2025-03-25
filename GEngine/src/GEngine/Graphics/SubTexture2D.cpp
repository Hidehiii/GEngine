#include "GEpch.h"
#include "SubTexture2D.h"

namespace GEngine
{
	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const Vector2& buttomLeft, const Vector2& topRight)
		: m_Texture(texture)
	{
		m_UV[0] = { buttomLeft.x, buttomLeft.y };
		m_UV[1] = { topRight.x, buttomLeft.y };
		m_UV[2] = { topRight.x, topRight.y };
		m_UV[3] = { buttomLeft.x, topRight.y };
	}
	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const Vector2& offset, const Vector2& cellSize, const Vector2& spriteSize, const Vector2& tiling)
	{
		// New UV
		Vector2 uv[2] =
		{
			{ (offset.x * cellSize.x) / texture->GetWidth(), (offset.y * cellSize.y) / texture->GetHeight()},
			{ ((offset.x + spriteSize.x) * cellSize.x) / texture->GetWidth(), ((offset.y + spriteSize.y) * cellSize.y) / texture->GetHeight()}
		};

		return CreateRef<SubTexture2D>(texture, uv[0], uv[1]);
	}
}
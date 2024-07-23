#include "GEpch.h"
#include "SubTexture2D.h"

namespace GEngine
{
	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const Vector2& buttomLeft, const Vector2& topRight)
		: m_Texture(texture)
	{
		m_UV[0] = { buttomLeft.value.x, buttomLeft.value.y };
		m_UV[1] = { topRight.value.x, buttomLeft.value.y };
		m_UV[2] = { topRight.value.x, topRight.value.y };
		m_UV[3] = { buttomLeft.value.x, topRight.value.y };
	}
	Ref<SubTexture2D> SubTexture2D::CreateFromCoords(const Ref<Texture2D>& texture, const Vector2& offset, const Vector2& cellSize, const Vector2& spriteSize, const Vector2& tiling)
	{
		// New UV
		Vector2 uv[2] =
		{
			{ (offset.value.x * cellSize.value.x) / texture->GetWidth(), (offset.value.y * cellSize.value.y) / texture->GetHeight()},
			{ ((offset.value.x + spriteSize.value.x) * cellSize.value.x) / texture->GetWidth(), ((offset.value.y + spriteSize.value.y) * cellSize.value.y) / texture->GetHeight()}
		};

		return CreateRef<SubTexture2D>(texture, uv[0], uv[1]);
	}
}
#pragma once

#include "GEngine/Core/Core.h"
#include "Texture.h"
#include "GEngine/Math/Math.h"

namespace GEngine
{
	class GENGINE_API SubTexture2D
	{
	public:
		SubTexture2D(const Ref<Texture2D>& texture, const Vector2& min, const Vector2& max);

		const Ref<Texture2D> GetTexture() const { return m_Texture; }
		const Vector2* GetUVs() const { return m_UV; }
		const std::string GetPath() const { return m_Texture->GetPath(); }

		static Ref<SubTexture2D> CreateFromCoords(const Ref<Texture2D>& texture, const Vector2& offset, const Vector2& cellSize, const Vector2& spriteSize = { 1.0f, 1.0f }, const Vector2& tiling = {1.0f, 1.0f});
	private:
		Ref<Texture2D> m_Texture;
		Vector2 m_UV[4];
	};
}



#pragma once

#include "GEngine/Core/Core.h"



namespace GEngine
{
	class GENGINE_API Serializer
	{
	public:
		Serializer() = default;
		~Serializer() = default;


		template<typename T>
		static void Serialize(const std::string& filepath, Ref<T>& data);

		template<typename T>
		static void Deserialize(const std::string& filepath, Ref<T>& data);
	};
}



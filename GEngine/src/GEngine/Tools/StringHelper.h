#pragma once
#include "GEngine/Core/Core.h"


namespace GEngine
{
	class GENGINE_API StringHelper
	{
	public:
		static std::string FromCharVector(const std::vector<char>& vector);

		static std::string ToLower(std::string string);
		static std::string ToUpper(std::string string);

		static std::vector<std::string> Split(const std::string& string, char delimiter);
		static std::string RemoveChar(const std::string& string, size_t pos, size_t len, char character);
	};
}



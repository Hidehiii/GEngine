#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
	class GENGINE_API FileSystemHelper
	{
	public:
		static bool IsFolder(const std::string& path);
		static bool CreateFolder(const std::string& path);
		static bool DeleteFolder(const std::string& path);

		static bool IsDocument(const std::string& path);
		static bool CreateDocument(const std::string& path);
		static bool DeleteDocument(const std::string& path);

		static std::vector<char> ReadFile(const std::string& path);
		static std::string ReadFileAsString(const std::string& path);

		static bool WriteFile(const std::string& path, const std::vector<char>& data);
		static bool WriteFile(const std::string& path, const std::string& data);
	};
}



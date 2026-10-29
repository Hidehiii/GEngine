#pragma once
#include "GEngine/Core/Core.h"
#include <filesystem>
#include <string>
#include <vector>

namespace GEngine
{
	struct GAssetEnvelope
	{
		std::string Type;
		std::string Format;
		std::vector<char> Payload;
	};

	GENGINE_API GAssetEnvelope LoadGAsset(const std::filesystem::path& path,
		const std::string& expectedType = {});
	GENGINE_API std::string LoadGAssetText(const std::filesystem::path& path,
		const std::string& expectedType = {});
	GENGINE_API std::vector<char> LoadGAssetBinary(const std::filesystem::path& path,
		const std::string& expectedType = {});
}

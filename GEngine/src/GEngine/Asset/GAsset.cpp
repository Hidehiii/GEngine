#include "GEpch.h"
#include "GEngine/Asset/GAsset.h"
#include "GEngine/Runtime/ResourcePack.h"
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

namespace GEngine
{
	namespace
	{
		bool IsValidIdentifier(const std::string& value)
		{
			if (value.empty()) return false;
			for (const auto character : value)
			{
				const bool alpha = (character >= 'a' && character <= 'z') ||
					(character >= 'A' && character <= 'Z');
				const bool numericOrSeparator = (character >= '0' && character <= '9') ||
					character == '-' || character == '_' || character == '.';
				if (!alpha && !numericOrSeparator) return false;
			}
			return true;
		}

		std::vector<char> ReadBytes(const std::filesystem::path& path)
		{
			const auto bytes = ReadResource(path);
			return bytes;
		}
	}

	GAssetEnvelope LoadGAsset(const std::filesystem::path& path, const std::string& expectedType)
	{
		if (path.extension() != ".gasset")
			throw std::runtime_error("GAsset path must use the .gasset extension: " + path.u8string());

		const auto bytes = ReadBytes(path);
		const auto document = std::string(bytes.begin(), bytes.end());
		YAML::Node root;
		try
		{
			root = YAML::Load(document);
		}
		catch (const std::exception& error)
		{
			throw std::runtime_error("Cannot parse GAsset YAML: " + path.u8string() + ": " + error.what());
		}

		if (!root.IsMap()) throw std::runtime_error("GAsset root must be a YAML map: " + path.u8string());
		for (const auto& item : root)
		{
			const auto key = item.first.as<std::string>();
			if (key != "schemaVersion" && key != "type" && key != "format" &&
				key != "payloadEncoding" && key != "payload")
				throw std::runtime_error("Unknown GAsset field '" + key + "': " + path.u8string());
		}
		if (!root["schemaVersion"]) throw std::runtime_error("GAsset schemaVersion is missing: " + path.u8string());
		if (!root["schemaVersion"].IsScalar() || root["schemaVersion"].as<int>() != 1)
			throw std::runtime_error("Unsupported GAsset schema version: " + path.u8string());
		if (!root["type"] || !root["type"].IsScalar() || !root["format"] || !root["format"].IsScalar())
			throw std::runtime_error("GAsset type/format is missing: " + path.u8string());
		if (!root["payloadEncoding"] || !root["payloadEncoding"].IsScalar() || !root["payload"])
			throw std::runtime_error("GAsset payload encoding/payload is missing: " + path.u8string());

		GAssetEnvelope envelope;
		envelope.Type = root["type"].as<std::string>();
		envelope.Format = root["format"].as<std::string>();
		const auto encoding = root["payloadEncoding"].as<std::string>();
		if (!IsValidIdentifier(envelope.Type) || !IsValidIdentifier(envelope.Format))
			throw std::runtime_error("GAsset type/format contains invalid characters: " + path.u8string());
		if (!expectedType.empty() && envelope.Type != expectedType)
			throw std::runtime_error("Unexpected GAsset type '" + envelope.Type + "', expected '" +
				expectedType + "': " + path.u8string());

		if (encoding == "text")
		{
			if (!root["payload"].IsScalar())
				throw std::runtime_error("GAsset text payload must be a scalar: " + path.u8string());
			const auto payload = root["payload"].as<std::string>();
			envelope.Payload.assign(payload.begin(), payload.end());
		}
		else if (encoding == "base64")
		{
			const auto scalar = root["payload"].Scalar();
			if (!root["payload"].IsScalar() || (!scalar.empty() && scalar.find_first_not_of(
				"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=\r\n") != std::string::npos))
				throw std::runtime_error("GAsset base64 payload is invalid: " + path.u8string());
			std::vector<unsigned char> decoded;
			if (!scalar.empty())
			{
				const auto binary = root["payload"].as<YAML::Binary>();
				decoded.assign(binary.data(), binary.data() + binary.size());
			}
			envelope.Payload.assign(decoded.begin(), decoded.end());
		}
		else
			throw std::runtime_error("Unsupported GAsset payload encoding '" + encoding + "': " + path.u8string());

		return envelope;
	}

	std::string LoadGAssetText(const std::filesystem::path& path, const std::string& expectedType)
	{
		const auto envelope = LoadGAsset(path, expectedType);
		return std::string(envelope.Payload.begin(), envelope.Payload.end());
	}

	std::vector<char> LoadGAssetBinary(const std::filesystem::path& path, const std::string& expectedType)
	{
		const auto envelope = LoadGAsset(path, expectedType);
		return envelope.Payload;
	}
}

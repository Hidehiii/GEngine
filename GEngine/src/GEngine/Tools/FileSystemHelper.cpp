#include "GEpch.h"
#include "FileSystemHelper.h"
#include <filesystem>

namespace GEngine
{
	bool FileSystemHelper::IsFolder(const std::string& path)
	{
		if (std::filesystem::exists(path))
		{
			if (std::filesystem::is_directory(path))
			{
				return true;
			}
		}
		else
		{
			GE_CORE_INFO("{0} not exists", path);
		}
		return false;
	}
	bool FileSystemHelper::CreateFolder(const std::string& path)
	{
		if (std::filesystem::exists(path))
		{
			GE_CORE_INFO("{0} already exists", path);
			return false;
		}
		std::filesystem::create_directories(path);
		return true;
	}

	bool FileSystemHelper::DeleteFolder(const std::string& path)
	{
		if (!std::filesystem::exists(path))
		{
			GE_CORE_INFO("{0} not exists", path);
			return false;
		}
		std::filesystem::remove_all(path);
		return true;
	}

	bool FileSystemHelper::IsDocument(const std::string& path)
	{
		if (std::filesystem::exists(path))
		{
			if (std::filesystem::is_regular_file(path))
			{
				return true;
			}
		}
		else
		{
			GE_CORE_INFO("{0} not exists", path);
		}
		return false;
	}

	bool FileSystemHelper::CreateDocument(const std::string& path)
	{
		if (std::filesystem::exists(path))
		{
			GE_CORE_INFO("{0} already exists", path);
			return false;
		}
		std::ofstream file(path);
		file.close();
		return true;
	}

	bool FileSystemHelper::DeleteDocument(const std::string& path)
	{
		if (!std::filesystem::exists(path))
		{
			GE_CORE_INFO("{0} not exists", path);
			return false;
		}
		std::filesystem::remove(path);
		return true;
	}

	std::vector<char> FileSystemHelper::ReadFile(const std::string& path)
	{
		std::vector<char> result;
		std::ifstream file(path, std::ios::in | std::ios::binary);

		if (!file)
		{
			GE_CORE_ERROR("file not found");
			return result;
		}

		file.seekg(0, std::ios::end);
		result.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(result.data(), result.size());
		file.close();

		return result;
	}

	std::string FileSystemHelper::ReadFileAsString(const std::string& path)
	{
		std::string str;
		std::ifstream file(path, std::ios::in | std::ios::binary);

		if (!file)
		{
			GE_CORE_ERROR("file not found");
			return str;
		}

		file.seekg(0, std::ios::end);
		str.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(&str[0], str.size());
		file.close();

		return str;
	}
	bool FileSystemHelper::WriteFile(const std::string& path, const std::vector<char>& data)
	{
		std::ofstream file(path, std::ios::out | std::ios::binary);

		if (!file)
		{
			GE_CORE_ERROR("file not found");
			return false;
		}

		file.write(data.data(), data.size());
		file.close();
		return true;
	}
	bool FileSystemHelper::WriteFile(const std::string& path, const std::string& data)
	{
		std::ofstream file(path, std::ios::out | std::ios::binary);

		if (!file)
		{
			GE_CORE_ERROR("file not found");
			return false;
		}

		file.write(data.c_str(), data.size());
		file.close();
		return true;
	}
}
#include "GEpch.h"
#include "GEngine/Tools/FileSystemHelper.h"
#include <filesystem>
#include "GEngine/Runtime/ResourcePack.h"

namespace GEngine
{
	bool FileSystemHelper::IsFolder(const std::string& path)
	{
		if (ResourceExists(path))
		{
			if (ResourceIsDirectory(path))
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
		EnsureResourceWritable(std::filesystem::u8path(path));
		if (std::filesystem::exists(path))
		{
			GE_CORE_INFO("{0} already exists", path);
			return false;
		}
		bool res = std::filesystem::create_directories(path);
		return res;
	}

	bool FileSystemHelper::DeleteFolder(const std::string& path)
	{
		EnsureResourceWritable(std::filesystem::u8path(path));
		if (!std::filesystem::exists(path))
		{
			GE_CORE_INFO("{0} not exists", path);
			return false;
		}
		std::filesystem::remove_all(path);
		return true;
	}

	std::vector<std::string> FileSystemHelper::GetFoldersInFolder(const std::string& folderPath, const std::string& partOfFolderName)
	{
		std::vector<std::string> result;
		if (IsFolder(folderPath))
		{
			for (const auto& entry : ListResourceDirectory(folderPath))
			{
				if (IsFolder(entry.string()))
				{
					if (partOfFolderName.empty() || entry.stem().string().find(partOfFolderName) != std::string::npos)
					{
						result.push_back(entry.string());
					}
				}
			}
			return result;
		}
		return std::vector<std::string>();
	}

	std::string FileSystemHelper::GetFolderName(const std::string& path)
	{
		if (IsFolder(path))
		{
			return std::filesystem::path(path).stem().string();
		}
		return std::string();
	}

	bool FileSystemHelper::IsDocument(const std::string& path)
	{
		if (ResourceExists(path))
		{
			if (!ResourceIsDirectory(path))
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
		EnsureResourceWritable(std::filesystem::u8path(path));
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
		EnsureResourceWritable(std::filesystem::u8path(path));
		if (!std::filesystem::exists(path))
		{
			GE_CORE_INFO("{0} not exists", path);
			return false;
		}
		std::filesystem::remove(path);
		return true;
	}

	std::vector<std::string> FileSystemHelper::GetDocumentsInFolder(const std::string& folderPath, const std::string& extension, const std::string& partOfDocName)
	{
		std::vector<std::string> result;
		if (IsFolder(folderPath))
		{
			for (const auto& entry : ListResourceDirectory(folderPath))
			{
				if (IsDocument(entry.string()))
				{
					if ((extension.empty() || entry.extension() == extension) &&
						(partOfDocName.empty() || entry.stem().string().find(partOfDocName) != std::string::npos))
					{
						result.push_back(entry.string());
					}
				}
			}
		}
		return result;
	}

	std::vector<std::string> FileSystemHelper::GetDocumentNamesInFolderWithExtension(const std::string& folderPath, const std::string& extension, const std::string& partOfDocName)
	{
		std::vector<std::string> result;
		if (IsFolder(folderPath))
		{
			for (const auto& entry : ListResourceDirectory(folderPath))
			{
				if (IsDocument(entry.string()))
				{
					if ((extension.empty() || entry.extension() == extension) &&
						(partOfDocName.empty() || entry.stem().string().find(partOfDocName) != std::string::npos))
					{
						result.push_back(entry.filename().string());
					}
				}
			}
		}
		return result;
	}

	std::vector<std::string> FileSystemHelper::GetDocumentNamesInFolderWithoutExtension(const std::string& folderPath, const std::string& extension, const std::string& partOfDocName)
	{
		std::vector<std::string> result;
		if (IsFolder(folderPath))
		{
			for (const auto& entry : ListResourceDirectory(folderPath))
			{
				if (IsDocument(entry.string()))
				{
					if ((extension.empty() || entry.extension() == extension) &&
						(partOfDocName.empty() || entry.stem().string().find(partOfDocName) != std::string::npos))
					{
						result.push_back(entry.stem().string());
					}
				}
			}
		}
		return result;
	}

	std::string FileSystemHelper::GetDocumentNameWithExtension(const std::string& path)
	{
		if (IsDocument(path))
		{
			return std::filesystem::path(path).filename().string();
		}
		return std::string();
	}

	std::string FileSystemHelper::GetDocumentNameWithoutExtension(const std::string& path)
	{
		if (IsDocument(path))
		{
			return std::filesystem::path(path).stem().string();
		}
		return std::string();
	}

	std::string FileSystemHelper::GetDocumentExtension(const std::string& path)
	{
		if (IsDocument(path))
		{
			return std::filesystem::path(path).extension().string();
		}
		return std::string();
	}

	std::vector<char> FileSystemHelper::ReadFile(const std::string& path)
	{
		return ReadResource(std::filesystem::u8path(path));
	}

	std::vector<unsigned char> FileSystemHelper::ReadFileAsBinary(const std::string& path)
	{
		const auto bytes = ReadFile(path);
		return std::vector<unsigned char>(bytes.begin(), bytes.end());
	}

	std::string FileSystemHelper::ReadFileAsString(const std::string& path)
	{
		const auto bytes = ReadFile(path);
		return std::string(bytes.begin(), bytes.end());
	}
	bool FileSystemHelper::WriteFile(const std::string& path, const std::vector<char>& data)
	{
		EnsureResourceWritable(std::filesystem::u8path(path));
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
	bool FileSystemHelper::WriteFile(const std::string& path, const std::vector<unsigned char>& data)
	{
		EnsureResourceWritable(std::filesystem::u8path(path));
		std::ofstream file(path, std::ios::out | std::ios::binary);

		if (!file)
		{
			GE_CORE_ERROR("file not found");
			return false;
		}

		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		file.close();
		return true;
	}
	bool FileSystemHelper::WriteFile(const std::string& path, const std::string& data)
	{
		EnsureResourceWritable(std::filesystem::u8path(path));
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

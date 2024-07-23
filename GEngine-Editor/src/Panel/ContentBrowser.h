#pragma once
#include <GEngine.h>
#include <filesystem>

namespace GEngine
{
	class ContentBrowser
	{
	public:
		ContentBrowser();
		~ContentBrowser() = default;

		void OnGuiRender();

	private:
		std::filesystem::path m_BaseDirectory;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_SelectedFile;
		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;

		// Assets preview
		Ref<Texture2D> m_SelectedFile_Texture;
	};
}



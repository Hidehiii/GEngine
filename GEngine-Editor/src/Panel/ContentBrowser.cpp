#include "ContentBrowser.h"
#include <imgui.h>


namespace GEngine
{
	ContentBrowser::ContentBrowser()
		: m_BaseDirectory(std::filesystem::path("Assets")), m_CurrentDirectory(m_BaseDirectory)
	{
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/folderIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/documentIcon.png");
	}

	void ContentBrowser::OnGuiRender()
	{
		ImGui::Begin("Content Browser");

		// Top bar
		std::filesystem::path tempPath = m_CurrentDirectory;
		std::vector<std::filesystem::path> paths;
		while (tempPath != m_BaseDirectory)
		{
			paths.emplace(paths.begin(), tempPath);
			tempPath = tempPath.parent_path();
		}
		paths.emplace(paths.begin(), m_BaseDirectory.filename());
		for (auto& path : paths)
		{
			if (ImGui::Button(path.filename().string().c_str()))
			{
				m_CurrentDirectory = path;
			}
			ImGui::SameLine();
			ImGui::Text(">>");
			ImGui::SameLine();
		}
		
		
		ImGui::NewLine();
		ImGui::Separator();

		static float padding = 10.0f;
		static float thumbnailSize = 100.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();

			ImGui::PushID(filenameString.c_str());
			Ref<Texture2D> icon;
			if (directoryEntry.is_directory())
			{
				icon = m_DirectoryIcon;
			}
			else
			{
				icon = m_FileIcon;
			}
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)GUIUtils::GetTextureID(icon), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
				{
					m_CurrentDirectory /= path.filename();
				}
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && directoryEntry.is_directory() == false)
			{
				m_SelectedFile = path;
				if (m_SelectedFile.extension() == ".png")
				{
					m_SelectedFile_Texture = Texture2D::Create(m_SelectedFile.string());
				}
				else
				{
					m_SelectedFile_Texture = nullptr;
				}
			}
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);
	
		// TODO: status bar
		ImGui::End();



		ImGui::Begin("Assets preview");
		if (m_SelectedFile != "")
		{
			ImGui::Text(m_SelectedFile.string().c_str());
			if (m_SelectedFile_Texture)
			{
				ImGui::Image((ImTextureID)GUIUtils::GetTextureID(m_SelectedFile_Texture), { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x }, {0.0f, 1.0f}, {1.0f, 0.0f});
			}
		}
		else
		{
			ImGui::Text("No file selected");
		}
		ImGui::End();
	}
}
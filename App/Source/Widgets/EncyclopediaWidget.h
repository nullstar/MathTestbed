#pragma once


#include "WindowWidget.h"
#include <imgui.h>
#include <imgui_markdown.h>
#include <filesystem>
#include <string>
#include <vector>



class EncyclopediaWidget : public IWindowWidget
{
public:
	EncyclopediaWidget(std::weak_ptr<MessageBus> pMessageBus);

private:
	struct EncyclopediaFile
	{
		std::filesystem::path m_path;
	};

	struct EncyclopediaFolder
	{
		std::filesystem::path m_path;
		std::vector<EncyclopediaFolder> m_folders;
		std::vector<EncyclopediaFile> m_files;
	};

	virtual void OnMessage(const MessageType& message) override;
	virtual const char* GetWindowName() const override;
	virtual void RenderContents(float deltaTime) override;
	
	static void LinkCallback(ImGui::MarkdownLinkCallbackData callbackData);
	static ImGui::MarkdownImageData ImageCallback(ImGui::MarkdownLinkCallbackData callbackData);

	ImGui::MarkdownConfig m_markdownConfig;
	EncyclopediaFolder m_rootFolder;
	const EncyclopediaFile* m_pSelectedFile = nullptr;
	const EncyclopediaFile* m_pRequestedFile = nullptr;
	std::string m_selectedFileContents;
};
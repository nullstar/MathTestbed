#include "EncyclopediaWidget.h"
#include <fstream>
#include <iostream>  
#include <sstream>  
#include <glm/glm.hpp>




EncyclopediaWidget::EncyclopediaWidget(std::weak_ptr<MessageBus> pMessageBus)
    : IWindowWidget(pMessageBus)
{
    // initialise markdown config
    m_markdownConfig.userData = this;
    m_markdownConfig.linkCallback = EncyclopediaWidget::LinkCallback;
    m_markdownConfig.imageCallback = EncyclopediaWidget::ImageCallback;

    // load encyclopedia data
    m_rootFolder.m_path = std::filesystem::current_path();
    m_rootFolder.m_path += std::filesystem::u8path("/Data/Encyclopedia");

    if (std::filesystem::exists(m_rootFolder.m_path) && std::filesystem::is_directory(m_rootFolder.m_path))
    {
        const auto loadFolder = [](EncyclopediaFolder& folder) -> void
        {
            const auto loadFolderImpl = [](EncyclopediaFolder& folder, auto& loadFolderRef) -> void
            {
                for (const auto& childEntry : std::filesystem::directory_iterator(folder.m_path))
                {
                    if (std::filesystem::is_directory(childEntry.status()))
                    {
                        folder.m_folders.push_back(EncyclopediaFolder());
                        EncyclopediaFolder& childFolder = folder.m_folders.back();
                        childFolder.m_path = childEntry.path();
                        loadFolderRef(childFolder, loadFolderRef);
                    }
                    else if (std::filesystem::is_regular_file(childEntry.status()))
                    {
                        const std::filesystem::path& childFilePath = childEntry.path();
                        if (childFilePath.has_extension() && childFilePath.extension() == ".md")
                        {
                            folder.m_files.push_back(EncyclopediaFile());
                            EncyclopediaFile& childFile = folder.m_files.back();
                            childFile.m_path = childFilePath;
                        }
                    }
                }
            };
            loadFolderImpl(folder, loadFolderImpl);
        };

        loadFolder(m_rootFolder);
    }
    else
    {
        std::cout << "ENCYCLOPEDIA DATA NOT FOUND" << std::endl;
    }
}


void EncyclopediaWidget::OnMessage(const MessageType& message)
{
    // determine message type from first token
    std::istringstream stream(message);
    std::string messageType;
    stream >> messageType;

    if (messageType == "GotoEncyclopediaPage")
    {
        std::string encyclopediaPageName;
        stream >> encyclopediaPageName;
        if (!encyclopediaPageName.empty())
        {
            const auto findPage = [&encyclopediaPageName](const EncyclopediaFolder& folder) -> const EncyclopediaFile*
            {
                const auto findPageImpl = [&encyclopediaPageName](const EncyclopediaFolder& folder, const auto& findPageRef) -> const EncyclopediaFile*
                {
                    for (const EncyclopediaFile& file : folder.m_files)
                    {
                        if (file.m_path.stem().generic_u8string() == encyclopediaPageName)
                            return &file;
                    }

                    for (const EncyclopediaFolder& childFolder : folder.m_folders)
                    {
                        if (const EncyclopediaFile* pFoundFile = findPageRef(childFolder, findPageRef))
                            return pFoundFile;
                    }

                    return nullptr;
                };
                return findPageImpl(folder, findPageImpl);
            };
            m_pRequestedFile = findPage(m_rootFolder);
        }
    }
}


const char* EncyclopediaWidget::GetWindowName() const
{
	return "Encyclopedia";
}


void EncyclopediaWidget::RenderContents(float deltaTime)
{
    // track which page we wish to display
    const EncyclopediaFile* pNextSelectedFile = m_pSelectedFile;
    if (m_pRequestedFile != nullptr)
    {
        pNextSelectedFile = m_pRequestedFile;
        m_pRequestedFile = nullptr;
    }

	// draw navigation pane
    ImGui::BeginChild("navigation pane", ImVec2(250, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    const auto drawDirectory = [](const EncyclopediaFolder& folder, const EncyclopediaFile* pSelectedFile) -> const EncyclopediaFile*
    {
        const auto drawDirectoryImpl = [](const EncyclopediaFolder& folder, const EncyclopediaFile* pSelectedFile, auto& drawDirectoryRef) -> const EncyclopediaFile*
        {
            for (const EncyclopediaFolder& childFolder : folder.m_folders)
            {
                constexpr ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
                if (ImGui::TreeNodeEx(static_cast<const void*>(&childFolder), nodeFlags, childFolder.m_path.stem().generic_u8string().c_str()))
                {
                    pSelectedFile = drawDirectoryRef(childFolder, pSelectedFile, drawDirectoryRef);
                    ImGui::TreePop();
                }
            }

            for (const EncyclopediaFile& childFile : folder.m_files)
            {
                ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;
                if (&childFile == pSelectedFile)
                    nodeFlags |= ImGuiTreeNodeFlags_Selected;

                ImGui::TreeNodeEx(static_cast<const void*>(pSelectedFile), nodeFlags, childFile.m_path.stem().generic_u8string().c_str());

                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                    pSelectedFile = &childFile;
            }

            return pSelectedFile;
        };
        return drawDirectoryImpl(folder, pSelectedFile, drawDirectoryImpl);
    };
    
    pNextSelectedFile = drawDirectory(m_rootFolder, pNextSelectedFile);
    ImGui::EndChild();

    // change selected file
    if (pNextSelectedFile != m_pSelectedFile)
    {
        m_pSelectedFile = pNextSelectedFile;
        m_selectedFileContents.clear();

        if (m_pSelectedFile != nullptr && std::filesystem::exists(m_pSelectedFile->m_path))
        {
            std::ifstream stream(m_pSelectedFile->m_path);
            std::stringstream buffer;
            buffer << stream.rdbuf();
            m_selectedFileContents = buffer.str();
        }

        Focus();
    }

    // draw markdown pane
    ImGui::SameLine();
    ImGui::BeginChild("markdown pane", ImVec2(0, 0), false);
    ImGui::Markdown(m_selectedFileContents.c_str(), m_selectedFileContents.length(), m_markdownConfig);
    ImGui::EndChild();
}


void EncyclopediaWidget::LinkCallback(ImGui::MarkdownLinkCallbackData callbackData)
{
    EncyclopediaWidget* pWidget = (EncyclopediaWidget*)callbackData.userData;
    std::string message(callbackData.link, callbackData.linkLength);
    pWidget->SendMessage(message);
}


ImGui::MarkdownImageData EncyclopediaWidget::ImageCallback(ImGui::MarkdownLinkCallbackData callbackData)
{
    // TODO
    return ImGui::MarkdownImageData();
}
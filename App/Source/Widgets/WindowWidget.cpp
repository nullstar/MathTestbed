#include "WindowWidget.h"
#include <imgui.h>



IWindowWidget::IWindowWidget()
{
}


IWindowWidget::~IWindowWidget()
{
}


void IWindowWidget::Render()
{
	// update focus
    if (m_wantsFocus)
    {
        ImGui::SetNextWindowCollapsed(false); 
        ImGui::SetNextWindowFocus(); 
        m_wantsFocus = false;
    }

    // draw window
    if (!ImGui::Begin(GetWindowName(), &m_isOpen))
    {
        ImGui::End();
        return;
    }

    // render contents
    RenderContents();

    ImGui::End();
}


void IWindowWidget::Focus()
{
    m_wantsFocus = true;
}


void IWindowWidget::Close()
{
    m_isOpen = false;
}


bool IWindowWidget::WantsToClose() const
{
    return !m_isOpen;
}
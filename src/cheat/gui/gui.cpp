#include "pch.h"

#include "gui.h"

void gui::drawgui()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();

    if (GetAsyncKeyState(m_iBind) & 0x1)
        m_bOpen = !m_bOpen;

    if (m_bOpen)
    {
        ImGui::Begin("ChudBase");

        ImGui::Text("wowowwowow");

        ImGui::End();
    }
}
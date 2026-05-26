#include "Theme.h"
#include <imgui.h>

void Theme::apply()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 8.0f;
    style.FrameRounding     = 4.0f;
    style.ItemSpacing       = {10.0f, 8.0f};
    style.FramePadding      = {8.0f, 5.0f};
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding      = 4.0f;

    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg]         = {0.08f, 0.08f, 0.08f, 1.00f};
    c[ImGuiCol_ChildBg]          = {0.10f, 0.10f, 0.10f, 1.00f};
    c[ImGuiCol_PopupBg]          = {0.10f, 0.10f, 0.10f, 1.00f};
    c[ImGuiCol_Border]           = {0.40f, 0.33f, 0.00f, 0.60f};
    c[ImGuiCol_FrameBg]          = {0.14f, 0.14f, 0.14f, 1.00f};
    c[ImGuiCol_FrameBgHovered]   = {0.20f, 0.18f, 0.05f, 1.00f};
    c[ImGuiCol_FrameBgActive]    = {0.25f, 0.22f, 0.05f, 1.00f};
    c[ImGuiCol_TitleBg]          = {0.05f, 0.20f, 0.05f, 1.00f};
    c[ImGuiCol_TitleBgActive]    = {0.06f, 0.28f, 0.06f, 1.00f};
    c[ImGuiCol_MenuBarBg]        = {0.06f, 0.06f, 0.06f, 1.00f};
    c[ImGuiCol_ScrollbarBg]      = {0.06f, 0.06f, 0.06f, 1.00f};
    c[ImGuiCol_ScrollbarGrab]    = {0.40f, 0.33f, 0.00f, 1.00f};
    c[ImGuiCol_Button]           = {0.06f, 0.28f, 0.06f, 1.00f};
    c[ImGuiCol_ButtonHovered]    = {0.08f, 0.40f, 0.08f, 1.00f};
    c[ImGuiCol_ButtonActive]     = {0.04f, 0.18f, 0.04f, 1.00f};
    c[ImGuiCol_Header]           = {0.06f, 0.25f, 0.06f, 1.00f};
    c[ImGuiCol_HeaderHovered]    = {0.08f, 0.35f, 0.08f, 1.00f};
    c[ImGuiCol_HeaderActive]     = {0.04f, 0.18f, 0.04f, 1.00f};
    c[ImGuiCol_Separator]        = {0.40f, 0.33f, 0.00f, 0.50f};
    c[ImGuiCol_Text]             = {0.95f, 0.90f, 0.75f, 1.00f};
    c[ImGuiCol_TextDisabled]     = {0.50f, 0.45f, 0.30f, 1.00f};
    c[ImGuiCol_CheckMark]        = {1.00f, 0.84f, 0.00f, 1.00f};
    c[ImGuiCol_SliderGrab]       = {1.00f, 0.84f, 0.00f, 1.00f};
    c[ImGuiCol_SliderGrabActive] = {0.75f, 0.60f, 0.00f, 1.00f};
    c[ImGuiCol_Tab]              = {0.05f, 0.20f, 0.05f, 1.00f};
    c[ImGuiCol_TabHovered]       = {0.08f, 0.35f, 0.08f, 1.00f};
    c[ImGuiCol_TabActive]        = {0.06f, 0.28f, 0.06f, 1.00f};
}

void Theme::PushTitleStyle()
{
    ImGui::PushStyleColor(ImGuiCol_Text, Gold());
}

void Theme::PopTitleStyle()
{
    ImGui::PopStyleColor();
}

void Theme::PushButtonGold()
{
    ImGui::PushStyleColor(ImGuiCol_Button,        {0.55f, 0.42f, 0.00f, 1.00f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.75f, 0.60f, 0.00f, 1.00f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.40f, 0.30f, 0.00f, 1.00f});
}

void Theme::PopButtonGold()
{
    ImGui::PopStyleColor(3);
}

void Theme::PushButtonRed()
{
    ImGui::PushStyleColor(ImGuiCol_Button,        {0.55f, 0.08f, 0.08f, 1.00f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.75f, 0.12f, 0.12f, 1.00f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.40f, 0.05f, 0.05f, 1.00f});
}

void Theme::PopButtonRed()
{
    ImGui::PopStyleColor(3);
}

#include "Theme.h"

#include "Wrapper/imgui.h"

namespace Cosmos
{
    void StyleColorsSpectrum()
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        style->GrabRounding = 4.0f;
        style->WindowMinSize = ImVec2(5.0f, 5.0f);
        style->Alpha = 1.0f;

        ImVec4* colors = style->Colors;
        colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(GRAY800); // text on hovered controls is gray900
        colors[ImGuiCol_TextDisabled] = ImGui::ColorConvertU32ToFloat4(GRAY500);
        colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(GRAY100);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(GRAY50); // not sure about this. Note: applies to tooltips too.
        colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(GRAY300);
        colors[ImGuiCol_BorderShadow] = ImGui::ColorConvertU32ToFloat4(Static::NONE); // We don't want shadows. Ever.
        colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(GRAY75); // this isnt right, spectrum does not do this, but it's a good fallback
        colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(GRAY50);
        colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(GRAY200);
        colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(GRAY300); // those titlebar values are totally made up, spectrum does not have this.
        colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(GRAY200);
        colors[ImGuiCol_TitleBgCollapsed] = ImGui::ColorConvertU32ToFloat4(GRAY400);
        colors[ImGuiCol_MenuBarBg] = ImGui::ColorConvertU32ToFloat4(GRAY100);
        colors[ImGuiCol_ScrollbarBg] = ImGui::ColorConvertU32ToFloat4(GRAY100); // same as regular background
        colors[ImGuiCol_ScrollbarGrab] = ImGui::ColorConvertU32ToFloat4(GRAY400);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImGui::ColorConvertU32ToFloat4(GRAY600);
        colors[ImGuiCol_ScrollbarGrabActive] = ImGui::ColorConvertU32ToFloat4(GRAY700);
        colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(BLUE500);
        colors[ImGuiCol_SliderGrab] = ImGui::ColorConvertU32ToFloat4(GRAY700);
        colors[ImGuiCol_SliderGrabActive] = ImGui::ColorConvertU32ToFloat4(GRAY800);
        colors[ImGuiCol_Button] = ImGui::ColorConvertU32ToFloat4(GRAY75); // match default button to Spectrum's 'Action Button'.
        colors[ImGuiCol_ButtonHovered] = ImGui::ColorConvertU32ToFloat4(GRAY50);
        colors[ImGuiCol_ButtonActive] = ImGui::ColorConvertU32ToFloat4(GRAY200);
        colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(BLUE400);
        colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(BLUE500);
        colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(BLUE600);
        colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(GRAY400);
        colors[ImGuiCol_SeparatorHovered] = ImGui::ColorConvertU32ToFloat4(GRAY600);
        colors[ImGuiCol_SeparatorActive] = ImGui::ColorConvertU32ToFloat4(GRAY700);
        colors[ImGuiCol_ResizeGrip] = ImGui::ColorConvertU32ToFloat4(GRAY400);
        colors[ImGuiCol_ResizeGripHovered] = ImGui::ColorConvertU32ToFloat4(GRAY600);
        colors[ImGuiCol_ResizeGripActive] = ImGui::ColorConvertU32ToFloat4(GRAY700);
        colors[ImGuiCol_PlotLines] = ImGui::ColorConvertU32ToFloat4(BLUE400);
        colors[ImGuiCol_PlotLinesHovered] = ImGui::ColorConvertU32ToFloat4(BLUE600);
        colors[ImGuiCol_PlotHistogram] = ImGui::ColorConvertU32ToFloat4(BLUE400);
        colors[ImGuiCol_PlotHistogramHovered] = ImGui::ColorConvertU32ToFloat4(BLUE600);
        colors[ImGuiCol_TextSelectedBg] = ImGui::ColorConvertU32ToFloat4((BLUE400 & 0x00FFFFFF) | 0x33000000);
        colors[ImGuiCol_DragDropTarget] = ImGui::ColorConvertU32ToFloat4(BLUE400);
        colors[ImGuiCol_NavHighlight] = ImGui::ColorConvertU32ToFloat4((GRAY900 & 0x00FFFFFF) | 0x0A000000);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    }
}
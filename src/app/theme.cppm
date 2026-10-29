module;

#include <imgui.h>

export module app.theme;

export namespace Theme {
void applyDarkTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* c = style.Colors;

    // Geometry
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;
    style.ScrollbarRounding = 8.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowMenuButtonPosition = ImGuiDir_None;

    // Palette
    const ImVec4 bg0 = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);
    const ImVec4 bg1 = ImVec4(0.10f, 0.10f, 0.11f, 1.00f);
    const ImVec4 bg2 = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);
    const ImVec4 bg3 = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    const ImVec4 bg4 = ImVec4(0.24f, 0.24f, 0.27f, 1.00f);
    const ImVec4 text = ImVec4(0.87f, 0.88f, 0.90f, 1.00f);
    const ImVec4 textDim = ImVec4(0.55f, 0.56f, 0.60f, 1.00f);
    const ImVec4 accent = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    // Text
    c[ImGuiCol_Text] = text;
    c[ImGuiCol_TextDisabled] = textDim;

    // Backgrounds
    c[ImGuiCol_WindowBg] = bg0;
    c[ImGuiCol_ChildBg] = bg1;
    c[ImGuiCol_PopupBg] = bg1;
    c[ImGuiCol_MenuBarBg] = bg1;

    // Frames / inputs
    c[ImGuiCol_FrameBg] = bg2;
    c[ImGuiCol_FrameBgHovered] = bg3;
    c[ImGuiCol_FrameBgActive] = bg4;

    // Titles
    c[ImGuiCol_TitleBg] = bg0;
    c[ImGuiCol_TitleBgActive] = bg1;

    // Buttons
    c[ImGuiCol_Button] = bg2;
    c[ImGuiCol_ButtonHovered] = bg3;
    c[ImGuiCol_ButtonActive] = bg4;

    // Accent bits
    c[ImGuiCol_CheckMark] = accent;
    c[ImGuiCol_SliderGrab] = accent;
    c[ImGuiCol_SliderGrabActive] = accent;
    c[ImGuiCol_Header] = bg3;
    c[ImGuiCol_HeaderHovered] = bg4;
    c[ImGuiCol_HeaderActive] = accent;

    // Tabs
    c[ImGuiCol_Tab] = bg1;
    c[ImGuiCol_TabHovered] = bg3;
    c[ImGuiCol_TabSelected] = bg2;

    // Borders / separators
    c[ImGuiCol_Border] = ImVec4(0.22f, 0.22f, 0.24f, 0.80f);
    c[ImGuiCol_Separator] = ImVec4(0.22f, 0.22f, 0.24f, 1.00f);

    // Scrollbar
    c[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ScrollbarGrab] = bg3;
    c[ImGuiCol_ScrollbarGrabHovered] = bg4;
    c[ImGuiCol_ScrollbarGrabActive] = accent;
}
}  // namespace Theme
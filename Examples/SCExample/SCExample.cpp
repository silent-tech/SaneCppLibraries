#include "../../Dependencies/imgui/_imgui/imgui.h"
#include "../../Libraries/UserInterface/Platform.h"

static bool      show_test_window    = true;
static bool      show_another_window = false;
extern "C" bool  sapp_is_fullscreen(void);
extern "C" void  sapp_toggle_fullscreen(void);
extern "C" int   sapp_width(void);
extern "C" int   sapp_height(void);
extern "C" float sapp_dpi_scale(void);
struct sg_color
{
    float r, g, b, a;
};
extern sg_color gBackgroundValue;

void platform_draw()
{
    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    static float f = 0.0f;
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::ColorEdit3("clear color", &gBackgroundValue.r);
    if (ImGui::Button("Test Window"))
        show_test_window ^= 1;
    if (ImGui::Button("Another Window"))
        show_another_window ^= 1;
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("w: %d, h: %d, dpi_scale: %.1f", sapp_width(), sapp_height(), sapp_dpi_scale());
    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen"))
    {
        sapp_toggle_fullscreen();
    }

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (show_another_window)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello");
        ImGui::End();
    }

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowDemoWindow()
    if (show_test_window)
    {
        ImGui::SetNextWindowPos(ImVec2(460, 20), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow();
    }
}
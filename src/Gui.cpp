#include "Gui.h"

#include "Window.h"
#include "Device.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_dx11.h>

Gui::~Gui()
{
    if (m_rendererReady)
        ImGui_ImplDX11_Shutdown();
    if (m_platformReady)
        ImGui_ImplGlfw_Shutdown();
    if (ImGui::GetCurrentContext())
        ImGui::DestroyContext();
}

bool Gui::Init(const Window& window, Device& device)
{
    m_device = &device;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    m_platformReady = ImGui_ImplGlfw_InitForOther(window.Handle(), true);
    if (!m_platformReady)
        return false;

    m_rendererReady = ImGui_ImplDX11_Init(device.Get(), device.Context());
    return m_rendererReady;
}

void Gui::BeginFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::Render()
{
    ImGui::Render();
    m_device->BeginUI();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

bool Gui::WantsMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

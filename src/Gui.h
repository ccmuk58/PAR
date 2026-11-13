#pragma once

class Window;
class Device;

class Gui
{
public:
    Gui() = default;
    ~Gui();

    Gui(const Gui&) = delete;
    Gui& operator=(const Gui&) = delete;

    bool Init(const Window& window, Device& device);

    void BeginFrame();
    void Render();

    bool WantsMouse() const;
    bool WantsTextInput() const;

private:
    Device* m_device = nullptr;
    bool    m_platformReady = false;
    bool    m_rendererReady = false;
};

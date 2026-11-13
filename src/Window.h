#pragma once
#include <windows.h>
#include <cstdint>
#include <functional>

struct GLFWwindow;

class Window
{
public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool Init(int width, int height, const char* title);

    // ---- 메인 루프용 ----
    void PollEvents();
    void WaitEvents();
    bool ShouldClose() const;
    bool IsMinimized() const;
    double Time() const;

    // ---- Device 생성에 필요한 정보 ----
    HWND     Hwnd() const;
    uint32_t Width() const;
    uint32_t Height() const;

    void SetResizeCallback(std::function<void(uint32_t, uint32_t)> callback);

private:
    GLFWwindow* m_window = nullptr;
    std::function<void(uint32_t, uint32_t)> m_onResize;
};

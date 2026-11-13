#include "Window.h"
#include "Device.h"
#include "Renderer.h"

int main()
{
    // 스왑체인이 HWND에 묶여 있어서, Window가 Device보다 오래 살아야 한다
    Window   window;
    Device   device;
    Renderer renderer;

    if (!window.Init(1280, 720, "PAR"))
        return -1;
    if (!device.Init(window.Hwnd(), window.Width(), window.Height()))
        return -1;
    if (!renderer.Init(device))
        return -1;

    window.SetResizeCallback([&device](uint32_t width, uint32_t height) {
        device.Resize(width, height);
    });

    // ---------------- 메인 루프 ----------------
    while (!window.ShouldClose())
    {
        // OS 이벤트 처리
        window.PollEvents();

        // 최소화 중 렌더링 미처리
        if (window.IsMinimized())
        {
            window.WaitEvents();
            continue;
        }

        const float clearColor[4] = { 0.02f, 0.02f, 0.03f, 1.0f };
        device.BeginFrame(clearColor);
        renderer.Render(static_cast<float>(window.Time()));
        device.Present(true);
    }

    return 0;
}   // Renderer -> Device -> Window 순으로 소멸

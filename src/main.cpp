#include "Window.h"
#include "Device.h"
#include "Renderer.h"
#include "Camera.h"
#include "CameraController.h"

int main()
{
    // 스왑체인이 HWND에 묶여 있어서, Window가 Device보다 오래 살아야 함
    Window   window;
    Device   device;
    Renderer renderer;
    Camera camera;
    CameraController cameraController;

    if (!window.Init(1280, 720, "PAR"))
        return -1;
    if (!device.Init(window.Hwnd(), window.Width(), window.Height()))
        return -1;
    if (!renderer.Init(device))
        return -1;

	// 카메라 초기화
	float initialY = 2.0f;
    camera.SetPosition({ 0.0f, initialY, -6.0f });
    camera.LookAt({ 0.0f, initialY, 0.0f });
    camera.SetPerspective(DirectX::XMConvertToRadians(60.0f), 0.1f, 100.0f);
    camera.SetAspectRatio(device.AspectRatio());

    window.SetCursorCaptured(true);

    window.SetResizeCallback([&device, &camera](uint32_t width, uint32_t height) {
        device.Resize(width, height);
        camera.SetAspectRatio(device.AspectRatio());
    });

    // ---------------- 메인 루프 ----------------
    double lastTime = window.Time();
    while (!window.ShouldClose())
    {
        // OS 이벤트 처리
        window.PollEvents();

        // 최소화 중 렌더링 미처리
        if (window.IsMinimized())
        {
            window.WaitEvents();
            lastTime = window.Time();   // 대기 시간 deltaTime 미반영
            continue;
        }

        // ESC: 커서 해제, 창 클릭: 커서 고정
        if (window.IsKeyDown(Key::Escape))
            window.SetCursorCaptured(false);
        else if (window.IsMouseButtonDown(MouseButton::Left))
            window.SetCursorCaptured(true);

        const double now = window.Time();
        const float deltaTime = static_cast<float>(now - lastTime);
        lastTime = now;

        cameraController.Update(camera, window, deltaTime);

        const float clearColor[4] = { 0.02f, 0.02f, 0.03f, 1.0f };
        device.BeginFrame(clearColor);
        renderer.Render(camera, static_cast<float>(now));
        device.Present(true);
    }

    return 0;
}   // Renderer -> Device -> Window 순으로 소멸

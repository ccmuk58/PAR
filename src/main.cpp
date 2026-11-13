#include "Window.h"
#include "Device.h"
#include "Renderer.h"
#include "Scene.h"
#include "Camera.h"
#include "CameraController.h"
#include "Gui.h"

#include <imgui.h>

namespace
{
    void DrawDebugPanel(Scene& scene, CameraController& cameraController)
    {
        const ImGuiIO& io = ImGui::GetIO();

        ImGui::Begin("PAR");
        ImGui::Text("%.1f FPS (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);
        ImGui::TextDisabled("ESC: release cursor / click viewport: capture");

        ImGui::SeparatorText("Light");
        DirectionalLight& light = scene.Light();
        DirectX::XMFLOAT3 direction = light.direction;
        if (ImGui::DragFloat3("Direction", &direction.x, 0.01f, -1.0f, 1.0f))
        {
            const float lengthSq = direction.x * direction.x + direction.y * direction.y + direction.z * direction.z;
            if (lengthSq > 1e-6f)
                light.direction = direction;
        }
        ImGui::ColorEdit3("Color", &light.color.x);
        ImGui::ColorEdit3("Ambient", &scene.AmbientColor().x);

        ImGui::SeparatorText("Camera");
        float moveSpeed = cameraController.MoveSpeed();
        if (ImGui::DragFloat("Move Speed", &moveSpeed, 0.05f, 0.1f, 50.0f))
            cameraController.SetMoveSpeed(moveSpeed);
        float sensitivity = cameraController.MouseSensitivity();
        if (ImGui::DragFloat("Sensitivity", &sensitivity, 0.0001f, 0.0001f, 0.01f, "%.4f"))
            cameraController.SetMouseSensitivity(sensitivity);

        ImGui::End();
    }
}

int main()
{
    // 스왑체인이 HWND에 묶여 있어서, Window가 Device보다 오래 살아야 함
    Window   window;
    Device   device;
    Renderer renderer;
    Scene    scene;
    Gui      gui;
    Camera camera;
    CameraController cameraController;

    if (!window.Init(1280, 720, "PAR"))
        return -1;
    if (!device.Init(window.Hwnd(), window.Width(), window.Height()))
        return -1;
    if (!renderer.Init(device))
        return -1;
    if (!scene.Init(device))
        return -1;
    if (!gui.Init(window, device))
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

        gui.BeginFrame();

        // ESC: 커서 해제, 뷰포트 클릭: 커서 고정
        if (window.IsKeyDown(Key::Escape))
            window.SetCursorCaptured(false);
        else if (window.IsMouseButtonDown(MouseButton::Left) && !gui.WantsMouse())
            window.SetCursorCaptured(true);

        const double now = window.Time();
        const float deltaTime = static_cast<float>(now - lastTime);
        lastTime = now;

        cameraController.Update(camera, window, deltaTime);
        scene.Update(static_cast<float>(now));
        DrawDebugPanel(scene, cameraController);

        const float clearColor[4] = { 0.02f, 0.02f, 0.03f, 1.0f };
        device.BeginFrame(clearColor);
        renderer.Render(scene, camera);
        gui.Render();
        device.Present(true);
    }

    return 0;
}   // Gui -> Scene -> Renderer -> Device -> Window 순으로 소멸

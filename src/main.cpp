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
    void DrawDebugPanel(Scene& scene, Renderer& renderer, CameraController& cameraController)
    {
        const ImGuiIO& io = ImGui::GetIO();

        ImGui::Begin("PAR");
        ImGui::Text("%.1f FPS (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);

        ImGui::SeparatorText("Light");
        DirectionalLight& light = scene.Light();
        DirectX::XMFLOAT3 direction = light.direction;
        if (ImGui::DragFloat3("Direction", &direction.x, 0.01f, -1.0f, 1.0f))
        {
            const float lengthSq = direction.x * direction.x + direction.y * direction.y + direction.z * direction.z;
            if (lengthSq > 1e-6f)
                light.direction = direction;
        }

        /*
        ImGui::ColorEdit3("Color", &light.color.x);
        ImGui::ColorEdit3("Ambient", &scene.AmbientColor().x);
        ImGui::SeparatorText("Point Light");
        ImGui::PushID("Point");
        PointLight& point = scene.Point();
        ImGui::DragFloat3("Position", &point.position.x, 0.05f);
        ImGui::ColorEdit3("Color", &point.color.x);
        ImGui::DragFloat("Intensity", &point.intensity, 0.05f, 0.0f, 20.0f);
        ImGui::DragFloat("Range", &point.range, 0.05f, 0.1f, 50.0f);
        ImGui::PopID();

        ImGui::SeparatorText("Spot Light");
        ImGui::PushID("Spot");
        SpotLight& spot = scene.Spot();
        ImGui::DragFloat3("Position", &spot.position.x, 0.05f);
        DirectX::XMFLOAT3 spotDirection = spot.direction;
        if (ImGui::DragFloat3("Direction", &spotDirection.x, 0.01f, -1.0f, 1.0f))
        {
            const float lengthSq = spotDirection.x * spotDirection.x + spotDirection.y * spotDirection.y + spotDirection.z * spotDirection.z;
            if (lengthSq > 1e-6f)
                spot.direction = spotDirection;
        }
        ImGui::ColorEdit3("Color", &spot.color.x);
        ImGui::DragFloat("Intensity", &spot.intensity, 0.05f, 0.0f, 20.0f);
        ImGui::DragFloat("Range", &spot.range, 0.05f, 0.1f, 50.0f);
        ImGui::DragFloat("Inner Angle", &spot.innerAngle, 0.1f, 0.0f, spot.outerAngle);
        ImGui::DragFloat("Outer Angle", &spot.outerAngle, 0.1f, spot.innerAngle, 89.0f);
        ImGui::PopID();


        ImGui::SeparatorText("Camera");
        float moveSpeed = cameraController.MoveSpeed();
        if (ImGui::DragFloat("Move Speed", &moveSpeed, 0.05f, 0.1f, 50.0f))
            cameraController.SetMoveSpeed(moveSpeed);
        float sensitivity = cameraController.MouseSensitivity();
        if (ImGui::DragFloat("Sensitivity", &sensitivity, 0.0001f, 0.0001f, 0.01f, "%.4f"))
            cameraController.SetMouseSensitivity(sensitivity);


        ImGui::SeparatorText("Tone Mapping");
        ToneMapSettings& toneMap = renderer.ToneMap();
        ImGui::SliderFloat("Exposure (EV)", &toneMap.exposure, -5.0f, 5.0f, "%.2f");
        int toneMapOp = static_cast<int>(toneMap.op);
        if (ImGui::Combo("Operator", &toneMapOp, "None\0Reinhard\0ACES\0"))
            toneMap.op = static_cast<ToneMapOperator>(toneMapOp);

        */


        ImGui::SeparatorText("Material");
        for (const auto& material : scene.Materials())
        {
            ImGui::PushID(material.get());
            if (ImGui::TreeNode(material->name.c_str()))
            {
                ImGui::ColorEdit3("Albedo", &material->albedo.x);
                ImGui::ColorEdit3("Specular", &material->specular.x);
                ImGui::DragFloat("Shininess", &material->shininess, 0.5f, 1.0f, 512.0f);
                ImGui::TreePop();
                ImGui::DragFloat("Height Scale", &material->heightScale, 0.001f, 0.0f, 0.2f, "%.3f");
            }
            ImGui::PopID();
        }
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
    bool escapeWasDown = false;
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

        // ESC: 커서 해제 (이미 해제 상태면 종료), 뷰포트 클릭: 커서 고정
        const bool escapeDown = window.IsKeyDown(Key::Escape);
        if (escapeDown && !escapeWasDown && !gui.WantsTextInput())
        {
            if (window.IsCursorCaptured())
                window.SetCursorCaptured(false);
            else
                window.RequestClose();
        }
        else if (!escapeDown && window.IsMouseButtonDown(MouseButton::Left) && !gui.WantsMouse())
            window.SetCursorCaptured(true);
        escapeWasDown = escapeDown;

        const double now = window.Time();
        const float deltaTime = static_cast<float>(now - lastTime);
        lastTime = now;

        cameraController.Update(camera, window, deltaTime);
        scene.Update(static_cast<float>(now));
        DrawDebugPanel(scene, renderer, cameraController);

        const float clearColor[4] = { 0.02f, 0.02f, 0.03f, 1.0f };
        device.BeginFrame(clearColor);
        renderer.Render(scene, camera, clearColor);
        gui.Render();
        device.Present(true);
    }

    return 0;
}   // Gui -> Scene -> Renderer -> Device -> Window 순으로 소멸

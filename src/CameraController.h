#pragma once

class Camera;
class Window;

// 1인칭 조작: 마우스로 시선 회전, WASD로 XZ 평면 이동
class CameraController
{
public:
    void Update(Camera& camera, const Window& window, float deltaTime) const;

    void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
    void SetMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }
    float MoveSpeed() const { return m_moveSpeed; }
    float MouseSensitivity() const { return m_mouseSensitivity; }

private:
    float m_moveSpeed = 3.0f;           // 초당 이동 거리
    float m_mouseSensitivity = 0.001f;  // 픽셀당 회전량 (라디안)
};

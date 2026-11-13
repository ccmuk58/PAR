#include "CameraController.h"

#include "Camera.h"
#include "Window.h"
#include <cmath>

void CameraController::Update(Camera& camera, const Window& window, float deltaTime) const
{
	if (!window.IsCursorCaptured()) return;
	
	const MouseDelta mouse = window.MouseMovement();
	camera.Rotate(mouse.x * m_mouseSensitivity, -mouse.y * m_mouseSensitivity);

	float forward = 0.0f, right = 0.0f, up = 0.0f;
	if (window.IsKeyDown(Key::W)) forward += 1.0f;
	if (window.IsKeyDown(Key::S)) forward -= 1.0f;
	if (window.IsKeyDown(Key::D)) right += 1.0f;
	if (window.IsKeyDown(Key::A)) right -= 1.0f;
	if (window.IsKeyDown(Key::Space)) up += 1.0f;
	if (window.IsKeyDown(Key::LeftShift)) up -= 1.0f;

	const float length = std::sqrt(forward * forward + right * right + up * up);
	if (length == 0.0f)
		return;

	const float sinYaw = std::sin(camera.Yaw());
	const float cosYaw = std::cos(camera.Yaw());
	const float step = m_moveSpeed * deltaTime / length;
	camera.Move({
		(forward * sinYaw + right * cosYaw) * step,
		up * step,
		(forward * cosYaw - right * sinYaw) * step });
}

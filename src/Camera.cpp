#include "Camera.h"

#include <algorithm>
#include <cmath>

using namespace DirectX;

constexpr float maxPitch = XMConvertToRadians(89.0f);

void Camera::LookAt(const XMFLOAT3& target)
{
	const XMVECTOR dir = XMLoadFloat3(&target) - XMLoadFloat3(&m_position);

	if (XMVector3Equal(dir, XMVectorZero()))
		return;

	XMFLOAT3 d;
	XMStoreFloat3(&d, XMVector3Normalize(dir));
	m_yaw = std::atan2(d.x, d.z);
	m_pitch = std::clamp(std::asin(std::clamp(d.y, -1.0f, 1.0f)), -maxPitch, maxPitch);
	UpdateForward();
}

void Camera::Move(const XMFLOAT3& offset)
{
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + XMLoadFloat3(&offset));
}

void Camera::Rotate(float yawDelta, float pitchDelta)
{
	m_yaw = XMScalarModAngle(m_yaw + yawDelta);
	m_pitch = std::clamp(m_pitch + pitchDelta, -maxPitch, maxPitch);
	UpdateForward();
}

void Camera::UpdateForward()
{
	const float cosPitch = std::cos(m_pitch);
	m_forward = { cosPitch * std::sin(m_yaw), std::sin(m_pitch), cosPitch * std::cos(m_yaw) };
}

void Camera::SetPerspective(float fovYRadians, float nearZ, float farZ)
{
	m_fovY = fovYRadians;
	m_nearZ = nearZ;
	m_farZ = farZ;
}

XMMATRIX Camera::View() const
{
	return XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_forward), XMLoadFloat3(&m_up));
}

XMMATRIX Camera::Proj() const
{
	return XMMatrixPerspectiveFovLH(m_fovY, m_aspect, m_nearZ, m_farZ);
}

XMMATRIX Camera::ViewProj() const
{
	return View() * Proj();
}

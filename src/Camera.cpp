#include "Camera.h"

using namespace DirectX;

void Camera::LookAt(const XMFLOAT3& target)
{
	const XMVECTOR dir = XMLoadFloat3(&target) - XMLoadFloat3(&m_position);

	if (XMVector3Equal(dir, XMVectorZero()))
		return;
	
	XMStoreFloat3(&m_forward, XMVector3Normalize(dir));
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

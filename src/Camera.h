#pragma once
#include <DirectXMath.h>

class Camera
{
public:
    // ---- 뷰 ----
    void SetPosition(const DirectX::XMFLOAT3& position) { m_position = position; }
    void LookAt(const DirectX::XMFLOAT3& target);

    // ---- 투영 ----
    void SetPerspective(float fovYRadians, float nearZ, float farZ);
    void SetAspectRatio(float aspect) { m_aspect = aspect; }

    // ---- 행렬 ----
    DirectX::XMMATRIX View() const;
    DirectX::XMMATRIX Proj() const;
    DirectX::XMMATRIX ViewProj() const;

    const DirectX::XMFLOAT3& Position() const { return m_position; }
    const DirectX::XMFLOAT3& Forward() const { return m_forward; }

private:
    DirectX::XMFLOAT3 m_position{ 0.0f, 0.0f, -3.0f };
    DirectX::XMFLOAT3 m_forward{ 0.0f, 0.0f, 1.0f };
    DirectX::XMFLOAT3 m_up{ 0.0f, 1.0f, 0.0f };

    float m_fovY = DirectX::XMConvertToRadians(60.0f);
    float m_aspect = 16.0f / 9.0f;
    float m_nearZ = 0.1f;
    float m_farZ = 100.0f;
};

#pragma once
#include "Mesh.h"
#include "Material.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>

class Device;

struct DirectionalLight
{
    // 빛이 진행하는 방향
    DirectX::XMFLOAT3 direction{ 0.5f, -0.7f, 0.6f };
    DirectX::XMFLOAT3 color{ 1.0f, 1.0f, 1.0f };
};

struct PointLight
{
    DirectX::XMFLOAT3 position{ -3.0f, 1.5f, -3.0f };
    DirectX::XMFLOAT3 color{ 1.0f, 0.45f, 0.15f };
    float intensity = 2.0f;
    float range = 8.0f;
};

struct SpotLight
{
    DirectX::XMFLOAT3 position{ 4.5f, 6.0f, -1.0f };
    DirectX::XMFLOAT3 direction{ -0.1f, -1.0f, 0.4f };
    DirectX::XMFLOAT3 color{ 0.2f, 0.5f, 1.0f };
    float intensity = 3.0f;
    float range = 14.0f;
    // 원뿔 반각 (degree)
    float innerAngle = 18.0f;
    float outerAngle = 26.0f;
};

struct SceneObject
{
    Mesh                      mesh;
    std::shared_ptr<Material> material;
    DirectX::XMFLOAT4X4 world{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f };
};

class Scene
{
public:
    bool Init(Device& device);
    void Update(float time);

    const std::vector<SceneObject>& Objects() const { return m_objects; }
    const std::vector<std::shared_ptr<Material>>& Materials() const { return m_materials; }
    const DirectionalLight& Light() const { return m_light; }
    const PointLight& Point() const { return m_pointLight; }
    const SpotLight& Spot() const { return m_spotLight; }
    const DirectX::XMFLOAT3& AmbientColor() const { return m_ambientColor; }

    DirectionalLight& Light() { return m_light; }
    PointLight& Point() { return m_pointLight; }
    SpotLight& Spot() { return m_spotLight; }
    DirectX::XMFLOAT3& AmbientColor() { return m_ambientColor; }

private:
    std::vector<SceneObject> m_objects;
    std::vector<std::shared_ptr<Material>> m_materials;
    DirectionalLight         m_light;
    PointLight               m_pointLight;
    SpotLight                m_spotLight;
    DirectX::XMFLOAT3        m_ambientColor{ 0.03f, 0.03f, 0.04f };
};

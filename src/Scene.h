#pragma once
#include "Mesh.h"
#include "Texture.h"
#include <DirectXMath.h>
#include <vector>

class Device;

struct DirectionalLight
{
    // 빛이 진행하는 방향
    DirectX::XMFLOAT3 direction{ 0.5f, -0.7f, 0.6f };
    DirectX::XMFLOAT3 color{ 1.0f, 1.0f, 1.0f };
};

struct SceneObject
{
    Mesh    mesh;
    Texture albedoMap;
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
    const DirectionalLight& Light() const { return m_light; }
    const DirectX::XMFLOAT3& AmbientColor() const { return m_ambientColor; }

private:
    std::vector<SceneObject> m_objects;
    DirectionalLight         m_light;
    DirectX::XMFLOAT3        m_ambientColor{ 0.03f, 0.03f, 0.04f };
};

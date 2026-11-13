#pragma once
#include "Vertex.h"
#include <d3d11_1.h>
#include <wrl/client.h>
#include <cstdint>
#include <filesystem>
#include <span>

class Device;

class Mesh
{
public:
    bool Init(Device& device, std::span<const Vertex> vertices, std::span<const uint32_t> indices);
    bool InitCube(Device& device);
    bool Load(Device& device, const std::filesystem::path& file);
    void Bind(ID3D11DeviceContext* ctx) const;
    UINT IndexCount() const { return m_indexCount; }

private:
    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;
    UINT m_indexCount = 0;
};
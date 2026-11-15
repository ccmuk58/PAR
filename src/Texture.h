#pragma once
#include <d3d11_1.h>
#include <wrl/client.h>
#include <cstdint>
#include <filesystem>
#include <span>

class Device;

class Texture
{
public:
    // pixels: RGBA8, width * height * 4 bytes
    bool Init(Device& device, uint32_t width, uint32_t height, std::span<const uint8_t> pixels, bool srgb = true);
    bool Load(Device& device, const std::filesystem::path& file, bool srgb = true);
    void Bind(ID3D11DeviceContext* ctx, UINT slot) const;
    bool IsValid() const { return m_srv != nullptr; }

private:
    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    ComPtr<ID3D11ShaderResourceView> m_srv;
};

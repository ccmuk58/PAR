#pragma once
#include <d3d11_1.h>
#include <wrl/client.h>
#include <filesystem>
#include <span>

class Device;

class Shader
{
public:
    bool Init(Device& device, const std::filesystem::path& file,
        std::span<const D3D11_INPUT_ELEMENT_DESC> layout);
    void Bind(ID3D11DeviceContext* ctx) const;

private:
    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    ComPtr<ID3D11VertexShader> m_vs;
    ComPtr<ID3D11PixelShader>  m_ps;
    ComPtr<ID3D11InputLayout>  m_inputLayout;
};
#pragma once
#include <d3d11_1.h> 
#include <dxgi1_2.h> 
#include <wrl/client.h>
#include <cstdint>

class Renderer
{
public:
    bool Init(HWND hwnd, uint32_t width, uint32_t height);
    void Resize(uint32_t width, uint32_t height);
    void Render();
    void Present(bool vsync);

private:
    bool CreateDevice();
    bool CreateSwapChain(HWND hwnd);
    bool CreateBackBufferViews();
    bool CreatePipeline();

    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    // ---- 핵심 ----
    ComPtr<ID3D11Device>           m_device;
    ComPtr<ID3D11DeviceContext>    m_context;
    ComPtr<IDXGISwapChain1>        m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_rtv;
    ComPtr<ID3D11DepthStencilView> m_dsv;

    // ---- 파이프라인 ----
    ComPtr<ID3D11VertexShader>     m_vs;
    ComPtr<ID3D11PixelShader>      m_ps;
    ComPtr<ID3D11InputLayout>      m_inputLayout;
    ComPtr<ID3D11Buffer>           m_vertexBuffer;

    uint32_t m_width  = 0;
    uint32_t m_height = 0;
};

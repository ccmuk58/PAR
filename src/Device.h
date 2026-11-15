#pragma once
#include <d3d11_1.h> 
#include <dxgi1_2.h> 
#include <wrl/client.h>
#include <cstdint>

class Device
{
public:
    bool Init(HWND hwnd, uint32_t width, uint32_t height);
    void Resize(uint32_t width, uint32_t height);
    void BeginFrame(const float clearColor[4]);
    void BindBackBuffer();
    void BeginUI();
    void Present(bool vsync);

    ID3D11Device* Get() const { return m_device.Get(); }
    ID3D11DeviceContext* Context() const { return m_context.Get(); }
    ID3D11DepthStencilView* DepthView() const { return m_dsv.Get(); }
    uint32_t Width() const { return m_width; }
    uint32_t Height() const { return m_height; }
    float AspectRatio() const { return float(m_width) / float(m_height); }

private:
    bool CreateDevice();
    bool CreateSwapChain(HWND hwnd);
    bool CreateBackBufferViews();

    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    ComPtr<ID3D11Device>           m_device;
    ComPtr<ID3D11DeviceContext>    m_context;
    ComPtr<IDXGISwapChain1>        m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_rtv;
    ComPtr<ID3D11RenderTargetView> m_rtvUI;
    ComPtr<ID3D11DepthStencilView> m_dsv;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
};
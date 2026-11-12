#pragma once
#include <d3d11_1.h> 
#include <wrl/client.h>

class Device;

class Renderer
{
public:
	bool Init(Device& device);
    void Render(float time);

private:
    bool CreatePipeline();

    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	Device* m_device = nullptr;

    ComPtr<ID3D11VertexShader>     m_vs;
    ComPtr<ID3D11PixelShader>      m_ps;
    ComPtr<ID3D11InputLayout>      m_inputLayout;
    ComPtr<ID3D11Buffer>           m_vertexBuffer;
    ComPtr<ID3D11Buffer>           m_cbPerFrame;
    ComPtr<ID3D11Buffer>           m_indexBuffer;
};

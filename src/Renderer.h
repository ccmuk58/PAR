#pragma once
#include "Shader.h"
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

    Shader m_shader;
    ComPtr<ID3D11Buffer>           m_vertexBuffer;
    ComPtr<ID3D11Buffer>           m_cbPerFrame;
    ComPtr<ID3D11Buffer>           m_indexBuffer;
};

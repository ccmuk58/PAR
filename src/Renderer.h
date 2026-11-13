#pragma once
#include "Shader.h"
#include <d3d11_1.h>
#include <wrl/client.h>

class Device;
class Camera;
class Scene;

class Renderer
{
public:
	bool Init(Device& device);
    void Render(const Scene& scene, const Camera& camera);

private:
    bool CreatePipeline();

    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	Device* m_device = nullptr;

    Shader                         m_shader;
    ComPtr<ID3D11SamplerState>     m_sampler;
    ComPtr<ID3D11Buffer>           m_cbPerFrame;
    ComPtr<ID3D11Buffer>           m_cbPerObject;
};

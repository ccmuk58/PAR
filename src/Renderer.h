#pragma once
#include "Shader.h"
#include "Mesh.h"
#include <d3d11_1.h> 
#include <wrl/client.h>

class Device;
class Camera;

class Renderer
{
public:
	bool Init(Device& device);
    void Render(const Camera& camera, float time);

private:
    bool CreatePipeline();

    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	Device* m_device = nullptr;

    Shader                         m_shader;
	Mesh                           m_cube;
    ComPtr<ID3D11Buffer>           m_cbPerFrame;
};

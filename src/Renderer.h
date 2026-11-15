#pragma once
#include "Shader.h"
#include "Material.h"
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
    bool CreateShadowMap();
    void DrawObjects(const Scene& scene, bool bindMaterial);
    void BindMaterial(const Material& material);

    template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	Device* m_device = nullptr;

    Shader                         m_shader;
    ComPtr<ID3D11SamplerState>     m_sampler;
    ComPtr<ID3D11Buffer>           m_cbPerFrame;
    ComPtr<ID3D11Buffer>           m_cbPerObject;
    ComPtr<ID3D11Buffer>           m_cbPerMaterial;

    Material m_defaultMaterial;
    Texture  m_whiteTexture;
    Texture  m_flatNormalTexture;

    Shader                           m_shadowShader;
    ComPtr<ID3D11DepthStencilView>   m_shadowDsv;
    ComPtr<ID3D11ShaderResourceView> m_shadowSrv;
    ComPtr<ID3D11SamplerState>       m_shadowSampler;
    ComPtr<ID3D11RasterizerState>    m_shadowRasterizer;
};

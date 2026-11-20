#pragma once
#include "Shader.h"
#include "Material.h"
#include "GpuProfiler.h"
#include <d3d11_1.h>
#include <wrl/client.h>

class Device;
class Camera;
class Scene;

enum class ToneMapOperator : int
{
    None,
    Reinhard,
    Aces,
};

struct ToneMapSettings
{
    float           exposure = 0.0f;
    ToneMapOperator op = ToneMapOperator::Aces;
};

class Renderer
{
public:
	bool Init(Device& device);
    void Render(const Scene& scene, const Camera& camera, const float clearColor[4]);

    ToneMapSettings& ToneMap() { return m_toneMap; }
    GpuProfiler&     Profiler() { return m_profiler; }

private:
    bool CreatePipeline();
    bool CreateShadowMap();
    bool CreateHdrTarget(UINT width, UINT height);
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

    Shader                           m_toneMapShader;
    ComPtr<ID3D11RenderTargetView>   m_hdrRtv;
    ComPtr<ID3D11ShaderResourceView> m_hdrSrv;
    ComPtr<ID3D11Buffer>             m_cbToneMap;
    ToneMapSettings                  m_toneMap;
    UINT                             m_hdrWidth = 0;
    UINT                             m_hdrHeight = 0;

    GpuProfiler m_profiler;
};

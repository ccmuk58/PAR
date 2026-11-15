#include "Renderer.h"
#include "Device.h"
#include "Camera.h"
#include "Scene.h"

#include "D3DUtil.h"
#include "Vertex.h"
#include <DirectXMath.h>
#include <filesystem>
#include <cmath>
#include <cstddef>
#include <cstring>

using Microsoft::WRL::ComPtr;
namespace
{
	constexpr UINT  kShadowMapSize = 2048;
	constexpr float kShadowRadius = 22.0f;

	struct alignas(16) PerFrameCB
	{
		DirectX::XMFLOAT4X4 viewProj;
		DirectX::XMFLOAT4X4 lightViewProj;
		DirectX::XMFLOAT3   cameraPos;
		float               pad0;
		DirectX::XMFLOAT3   lightDir;
		float               pad1;
		DirectX::XMFLOAT3   lightColor;
		float               pad2;
		DirectX::XMFLOAT3   ambientColor;
		float               pad3;

		DirectX::XMFLOAT3   pointPos;
		float               pointRange;
		DirectX::XMFLOAT3   pointColor;
		float               pad4;

		DirectX::XMFLOAT3   spotPos;
		float               spotRange;
		DirectX::XMFLOAT3   spotDir;
		float               spotCosInner;
		DirectX::XMFLOAT3   spotColor;
		float               spotCosOuter;
	};

	struct alignas(16) PerObjectCB
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 worldInvTranspose;
	};

	struct alignas(16) PerMaterialCB
	{
		DirectX::XMFLOAT3 albedo;
		float             shininess;
		DirectX::XMFLOAT3 specular;
		float             pad0;
	};

	struct alignas(16) ToneMapCB
	{
		float exposure;
		int   op;
		float pad[2];
	};

	static_assert(sizeof(PerMaterialCB) % 16 == 0, "cbuffer 크기가 16바이트의 배수가 아닙니다.");
	static_assert(sizeof(PerFrameCB) % 16 == 0, "cbuffer 크기가 16바이트의 배수가 아닙니다.");
	static_assert(sizeof(PerObjectCB) % 16 == 0, "cbuffer 크기가 16바이트의 배수가 아닙니다.");

	bool CreateConstantBuffer(ID3D11Device* dev, UINT size, ID3D11Buffer** buffer)
	{
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = size;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		return Check(dev->CreateBuffer(&desc, nullptr, buffer), "CreateBuffer(cb)");
	}

	template <class T>
	void UpdateConstantBuffer(ID3D11DeviceContext* ctx, ID3D11Buffer* buffer, const T& data)
	{
		D3D11_MAPPED_SUBRESOURCE mapped{};
		if (SUCCEEDED(ctx->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
		{
			std::memcpy(mapped.pData, &data, sizeof(T));
			ctx->Unmap(buffer, 0);
		}
	}

	DirectX::XMMATRIX ComputeLightViewProj(const DirectX::XMFLOAT3& lightDir)
	{
		using namespace DirectX;
		const XMVECTOR dir = XMVector3Normalize(XMLoadFloat3(&lightDir));
		const XMVECTOR eye = XMVectorScale(dir, -kShadowRadius);
		const XMVECTOR up = std::abs(XMVectorGetY(dir)) > 0.99f
			? XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f)
			: XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		const XMMATRIX view = XMMatrixLookToLH(eye, dir, up);
		const XMMATRIX proj = XMMatrixOrthographicLH(2.0f * kShadowRadius, 2.0f * kShadowRadius, 0.0f, 2.0f * kShadowRadius);
		return view * proj;
	}

} // namespace

bool Renderer::Init(Device& device)
{
	m_device = &device;
	return CreatePipeline() && CreateShadowMap() && CreateHdrTarget(device.Width(), device.Height());
}

bool Renderer::CreatePipeline()
{
	ID3D11Device* dev = m_device->Get();
	const auto shaderPath = std::filesystem::path(SHADER_DIR) / "Basic.hlsl";

	const D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(Vertex, uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	// shader 초기화
	if (!m_shader.Init(*m_device, shaderPath, layout))
		return false;
	if (!m_shadowShader.Init(*m_device, std::filesystem::path(SHADER_DIR) / "Shadow.hlsl", layout, true))
		return false;
	if (!m_toneMapShader.Init(*m_device, std::filesystem::path(SHADER_DIR) / "ToneMap.hlsl", {}))
		return false;

	// sampler 생성
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (!Check(dev->CreateSamplerState(&samplerDesc, &m_sampler), "CreateSamplerState"))
		return false;

	// constant buffer 생성
	if (!CreateConstantBuffer(dev, sizeof(PerFrameCB), &m_cbPerFrame))
		return false;
	if (!CreateConstantBuffer(dev, sizeof(PerObjectCB), &m_cbPerObject))
		return false;
	if (!CreateConstantBuffer(dev, sizeof(PerMaterialCB), &m_cbPerMaterial))
		return false;
	if (!CreateConstantBuffer(dev, sizeof(ToneMapCB), &m_cbToneMap))
		return false;

	// 기본 texture 생성
	const uint8_t white[4] = { 255, 255, 255, 255 };
	if (!m_whiteTexture.Init(*m_device, 1, 1, white))
		return false;
	const uint8_t flatNormal[4] = { 128, 128, 255, 255 };
	if (!m_flatNormalTexture.Init(*m_device, 1, 1, flatNormal, false))
		return false;

	return true;
}

bool Renderer::CreateShadowMap()
{
	ID3D11Device* dev = m_device->Get();

	// shadow map 생성
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = kShadowMapSize;
	desc.Height = kShadowMapSize;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> texture;
	if (!Check(dev->CreateTexture2D(&desc, nullptr, &texture), "CreateTexture2D(shadow)"))
		return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	if (!Check(dev->CreateDepthStencilView(texture.Get(), &dsvDesc, &m_shadowDsv), "CreateDepthStencilView(shadow)"))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	if (!Check(dev->CreateShaderResourceView(texture.Get(), &srvDesc, &m_shadowSrv), "CreateShaderResourceView(shadow)"))
		return false;

	// comparison sampler 생성
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if (!Check(dev->CreateSamplerState(&samplerDesc, &m_shadowSampler), "CreateSamplerState(shadow)"))
		return false;

	// depth bias rasterizer 생성
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 1000;
	rasterizerDesc.SlopeScaledDepthBias = 2.5f;
	rasterizerDesc.DepthClipEnable = TRUE;
	return Check(dev->CreateRasterizerState(&rasterizerDesc, &m_shadowRasterizer), "CreateRasterizerState(shadow)");
}

bool Renderer::CreateHdrTarget(UINT width, UINT height)
{
	ID3D11Device* dev = m_device->Get();
	m_hdrRtv.Reset();
	m_hdrSrv.Reset();

	// HDR render target 생성
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> texture;
	if (!Check(dev->CreateTexture2D(&desc, nullptr, &texture), "CreateTexture2D(hdr)"))
		return false;
	if (!Check(dev->CreateRenderTargetView(texture.Get(), nullptr, &m_hdrRtv), "CreateRenderTargetView(hdr)"))
		return false;
	if (!Check(dev->CreateShaderResourceView(texture.Get(), nullptr, &m_hdrSrv), "CreateShaderResourceView(hdr)"))
		return false;

	m_hdrWidth = width;
	m_hdrHeight = height;
	return true;
}

void Renderer::Render(const Scene& scene, const Camera& camera, const float clearColor[4])
{
	ID3D11DeviceContext* ctx = m_device->Context();
	using namespace DirectX;

	const UINT width = m_device->Width();
	const UINT height = m_device->Height();
	if ((width != m_hdrWidth || height != m_hdrHeight) && !CreateHdrTarget(width, height))
		return;

	// per frame constant buffer
	PerFrameCB frame{};
	XMStoreFloat4x4(&frame.viewProj, XMMatrixTranspose(camera.ViewProj()));
	XMStoreFloat4x4(&frame.lightViewProj, XMMatrixTranspose(ComputeLightViewProj(scene.Light().direction)));
	frame.cameraPos = camera.Position();
	frame.lightDir = scene.Light().direction;
	frame.lightColor = scene.Light().color;
	frame.ambientColor = scene.AmbientColor();

	const PointLight& point = scene.Point();
	frame.pointPos = point.position;
	frame.pointRange = point.range;
	XMStoreFloat3(&frame.pointColor, XMVectorScale(XMLoadFloat3(&point.color), point.intensity));

	const SpotLight& spot = scene.Spot();
	frame.spotPos = spot.position;
	frame.spotRange = spot.range;
	XMStoreFloat3(&frame.spotDir, XMVector3Normalize(XMLoadFloat3(&spot.direction)));
	XMStoreFloat3(&frame.spotColor, XMVectorScale(XMLoadFloat3(&spot.color), spot.intensity));
	frame.spotCosInner = std::cos(XMConvertToRadians(spot.innerAngle));
	frame.spotCosOuter = std::cos(XMConvertToRadians(spot.outerAngle));
	UpdateConstantBuffer(ctx, m_cbPerFrame.Get(), frame);

	ID3D11Buffer* const cbs[] = { m_cbPerFrame.Get(), m_cbPerObject.Get(), m_cbPerMaterial.Get() };
	ctx->VSSetConstantBuffers(0, 2, cbs);
	ctx->PSSetConstantBuffers(0, 3, cbs);

	// shadow pass
	const D3D11_VIEWPORT shadowViewport{ 0.0f, 0.0f, static_cast<float>(kShadowMapSize), static_cast<float>(kShadowMapSize), 0.0f, 1.0f };
	ctx->OMSetRenderTargets(0, nullptr, m_shadowDsv.Get());
	ctx->ClearDepthStencilView(m_shadowDsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	ctx->RSSetViewports(1, &shadowViewport);
	ctx->RSSetState(m_shadowRasterizer.Get());
	m_shadowShader.Bind(ctx);
	DrawObjects(scene, false);

	// main pass
	const D3D11_VIEWPORT viewport{ 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
	ctx->OMSetRenderTargets(1, m_hdrRtv.GetAddressOf(), m_device->DepthView());
	ctx->ClearRenderTargetView(m_hdrRtv.Get(), clearColor);
	ctx->RSSetViewports(1, &viewport);
	ctx->RSSetState(nullptr);
	m_shader.Bind(ctx);
	ID3D11SamplerState* const samplers[] = { m_sampler.Get(), m_shadowSampler.Get() };
	ctx->PSSetSamplers(0, 2, samplers);
	ctx->PSSetShaderResources(1, 1, m_shadowSrv.GetAddressOf());
	DrawObjects(scene, true);

	ID3D11ShaderResourceView* const nullSrv = nullptr;
	ctx->PSSetShaderResources(1, 1, &nullSrv);

	// tone mapping pass
	ToneMapCB toneMap{};
	toneMap.exposure = std::exp2(m_toneMap.exposure);
	toneMap.op = static_cast<int>(m_toneMap.op);
	UpdateConstantBuffer(ctx, m_cbToneMap.Get(), toneMap);

	m_device->BindBackBuffer();
	m_toneMapShader.Bind(ctx);
	ctx->PSSetConstantBuffers(0, 1, m_cbToneMap.GetAddressOf());
	ctx->PSSetShaderResources(0, 1, m_hdrSrv.GetAddressOf());
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->Draw(3, 0);
	ctx->PSSetShaderResources(0, 1, &nullSrv);
}

void Renderer::DrawObjects(const Scene& scene, bool bindMaterial)
{
	ID3D11DeviceContext* ctx = m_device->Context();
	using namespace DirectX;

	const Material* boundMaterial = nullptr;
	for (const SceneObject& sceneObject : scene.Objects())
	{
		// per object constant buffer
		const XMMATRIX world = XMLoadFloat4x4(&sceneObject.world);
		const XMMATRIX worldInvTranspose = XMMatrixTranspose(XMMatrixInverse(nullptr, world));
		PerObjectCB object{};
		XMStoreFloat4x4(&object.world, XMMatrixTranspose(world));
		XMStoreFloat4x4(&object.worldInvTranspose, XMMatrixTranspose(worldInvTranspose));
		UpdateConstantBuffer(ctx, m_cbPerObject.Get(), object);

		sceneObject.mesh.Bind(ctx);
		if (bindMaterial)
		{
			const Material* material = sceneObject.material ? sceneObject.material.get() : &m_defaultMaterial;
			if (material != boundMaterial)
			{
				BindMaterial(*material);
				boundMaterial = material;
			}
		}
		ctx->DrawIndexed(sceneObject.mesh.IndexCount(), 0, 0);
	}
}

void Renderer::BindMaterial(const Material& material)
{
	ID3D11DeviceContext* ctx = m_device->Context();

	// per material constant buffer
	PerMaterialCB data{};
	data.albedo = material.albedo;
	data.shininess = material.shininess;
	data.specular = material.specular;
	UpdateConstantBuffer(ctx, m_cbPerMaterial.Get(), data);

	(material.albedoMap.IsValid() ? material.albedoMap : m_whiteTexture).Bind(ctx, 0);
	(material.normalMap.IsValid() ? material.normalMap : m_flatNormalTexture).Bind(ctx, 2);
}

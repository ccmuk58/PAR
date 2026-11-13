#include "Renderer.h"
#include "Device.h"
#include "Camera.h"
#include "Scene.h"

#include "D3DUtil.h"
#include "Vertex.h"
#include <DirectXMath.h>
#include <filesystem>
#include <cstddef>
#include <cstring>

using Microsoft::WRL::ComPtr;
namespace
{
	struct alignas(16) PerFrameCB
	{
		DirectX::XMFLOAT4X4 viewProj;
		DirectX::XMFLOAT3   cameraPos;
		float               pad0;
		DirectX::XMFLOAT3   lightDir;
		float               pad1;
		DirectX::XMFLOAT3   lightColor;
		float               pad2;
		DirectX::XMFLOAT3   ambientColor;
		float               pad3;
	};

	struct alignas(16) PerObjectCB
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 worldInvTranspose;
	};

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

} // namespace

bool Renderer::Init(Device& device)
{
	m_device = &device;
	return CreatePipeline();
}

bool Renderer::CreatePipeline()
{
	ID3D11Device* dev = m_device->Get();
	const auto shaderPath = std::filesystem::path(SHADER_DIR) / "Basic.hlsl";

	const D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(Vertex, uv),     D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	// shader 초기화
	if (!m_shader.Init(*m_device, shaderPath, layout))
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

	return true;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	ID3D11DeviceContext* ctx = m_device->Context();
	using namespace DirectX;

	// per frame constant buffer
	PerFrameCB frame{};
	XMStoreFloat4x4(&frame.viewProj, XMMatrixTranspose(camera.ViewProj()));
	frame.cameraPos = camera.Position();
	frame.lightDir = scene.Light().direction;
	frame.lightColor = scene.Light().color;
	frame.ambientColor = scene.AmbientColor();
	UpdateConstantBuffer(ctx, m_cbPerFrame.Get(), frame);

	ID3D11Buffer* const cbs[] = { m_cbPerFrame.Get(), m_cbPerObject.Get() };
	m_shader.Bind(ctx);
	ctx->PSSetSamplers(0, 1, m_sampler.GetAddressOf());
	ctx->VSSetConstantBuffers(0, 2, cbs);
	ctx->PSSetConstantBuffers(0, 2, cbs);

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
		sceneObject.albedoMap.Bind(ctx, 0);
		ctx->DrawIndexed(sceneObject.mesh.IndexCount(), 0, 0);
	}
}

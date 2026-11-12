#include "Renderer.h"
#include "Device.h"

#include "D3DUtil.h"
#include "Vertex.h"
#include <DirectXMath.h>
#include <filesystem>
#include <cstddef>

using Microsoft::WRL::ComPtr;
namespace
{
	struct alignas(16) PerFrameCB
	{
		DirectX::XMFLOAT4X4 worldViewProj;
	};

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
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	// shader 초기화
	if (!m_shader.Init(*m_device, shaderPath, layout))
		return false;
	// mesh 초기화
	if (!m_cube.InitCube(*m_device))
		return false;

	// constant buffer 생성
	static_assert(sizeof(PerFrameCB) % 16 == 0, "cbuffer 크기가 16바이트의 배수가 아닙니다.");
	D3D11_BUFFER_DESC cbd{};
	cbd.ByteWidth = sizeof(PerFrameCB);
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (!Check(dev->CreateBuffer(&cbd, nullptr, &m_cbPerFrame), "CreateBuffer(cb)"))
		return false;

	return true;
}

void Renderer::Render(float time)
{
	ID3D11DeviceContext* ctx = m_device->Context();
	// wvp 행렬 계산
	using namespace DirectX;
	const float aspect = m_device->AspectRatio();
	XMMATRIX world = XMMatrixRotationY(time);
	XMMATRIX view = XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f),   // 카메라 위치
		XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),   // 바라볼 지점
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));  // 위쪽 방향
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), aspect, 0.1f, 100.0f);
	XMMATRIX wvp = world * view * proj;

	D3D11_MAPPED_SUBRESOURCE mapped{};
	if (SUCCEEDED(ctx->Map(m_cbPerFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
	{
		auto* cb = static_cast<PerFrameCB*>(mapped.pData);
		XMStoreFloat4x4(&cb->worldViewProj, XMMatrixTranspose(wvp));  // 전치
		ctx->Unmap(m_cbPerFrame.Get(), 0);
	}

	const float clearColor[4] = { 0.02f, 0.02f, 0.03f, 1.0f };
	m_device->BeginFrame(clearColor);

	m_shader.Bind(ctx);
	m_cube.Bind(ctx);
	ctx->VSSetConstantBuffers(0, 1, m_cbPerFrame.GetAddressOf());
	ctx->DrawIndexed(m_cube.IndexCount(), 0, 0);
}


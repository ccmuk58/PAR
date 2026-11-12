#include "Renderer.h"
#include "Device.h"

#include "D3DUtil.h"
#include <DirectXMath.h>
#include <filesystem>
#include <iterator>
#include <cstddef>

using Microsoft::WRL::ComPtr;
namespace
{
	struct Vertex
	{
		float pos[3];
		float color[3];
	};
	struct alignas(16) PerFrameCB
	{
		DirectX::XMFLOAT4X4 worldViewProj;
	};

	const Vertex vertices[] = {
	{ { -0.5f,-0.5f,-0.5f },{ 0,0,0 } }, // 0
	{ { -0.5f, 0.5f,-0.5f },{ 0,1,0 } }, // 1
	{ { 0.5f, 0.5f,-0.5f },{ 1,1,0 } }, // 2
	{ { 0.5f,-0.5f,-0.5f },{ 1,0,0 } }, // 3
	{ { -0.5f,-0.5f, 0.5f },{ 0,0,1 } }, // 4
	{ { -0.5f, 0.5f, 0.5f },{ 0,1,1 } }, // 5
	{ { 0.5f, 0.5f, 0.5f },{ 1,1,1 } }, // 6
	{ { 0.5f,-0.5f, 0.5f },{ 1,0,1 } }, // 7
	};
	const uint16_t indices[] = {
		0,1,2, 0,2,3,   // 앞 (-Z)
		7,6,5, 7,5,4,   // 뒤 (+Z)
		4,5,1, 4,1,0,   // 왼 (-X)
		3,2,6, 3,6,7,   // 오른 (+X)
		1,5,6, 1,6,2,   // 위 (+Y)
		3,7,4, 3,4,0,   // 아래 (-Y)
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
	if (!m_shader.Init(*m_device, shaderPath, layout))
		return false;

	// vertex buffer 생성
	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(vertices);
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA init{};
	init.pSysMem = vertices;
	if (!Check(dev->CreateBuffer(&bd, &init, &m_vertexBuffer), "CreateBuffer(vertex)"))
		return false;
	
	// index buffer 생성
	D3D11_BUFFER_DESC ibd{};
	ibd.ByteWidth = sizeof(indices);
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA iinit{};
	iinit.pSysMem = indices;
	if (!Check(dev->CreateBuffer(&ibd, &iinit, &m_indexBuffer), "CreateBuffer(index)"))
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

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	ctx->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_shader.Bind(ctx);

	ctx->VSSetConstantBuffers(0, 1, m_cbPerFrame.GetAddressOf());

	ctx->DrawIndexed(static_cast<UINT>(std::size(indices)), 0, 0);
}


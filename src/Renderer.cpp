#include "Renderer.h"

#include <d3dcompiler.h> 
#include <d3d11sdklayers.h> 
#include <cstddef> 
#include <cstdio>
#include <filesystem>
#include <iterator>

using Microsoft::WRL::ComPtr;

namespace
{
	struct Vertex
	{
		float pos[3];
		float color[3];
	};

	bool Check(HRESULT hr, const char* what)
	{
		if (FAILED(hr))
		{
			std::fprintf(stderr, "[D3D11] %s failed (0x%08X)\n", what, static_cast<unsigned>(hr));
			return false;
		}
		return true;
	}

	// HLSL을 Blob으로 컴파일
	ComPtr<ID3DBlob> CompileShader(const std::filesystem::path& file, const char* entry, const char* target)
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		ComPtr<ID3DBlob> code, errors;
		HRESULT hr = D3DCompileFromFile(file.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entry, target, flags, 0, &code, &errors);
		if (FAILED(hr))
		{
			if (errors)
				std::fprintf(stderr, "%s\n", static_cast<const char*>(errors->GetBufferPointer()));
			else
				std::fprintf(stderr, "Shader load failed: %ls (0x%08X)\n", file.c_str(), static_cast<unsigned>(hr));
			return nullptr;
		}
		return code;
	}
} // namespace

bool Renderer::Init(HWND hwnd, uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;
	return CreateDevice() && CreateSwapChain(hwnd) && CreateBackBufferViews() && CreatePipeline();
}

bool Renderer::CreateDevice()
{
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	const D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
		levels, static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION, &m_device, nullptr, &m_context);

#if defined(_DEBUG)
	if (hr == DXGI_ERROR_SDK_COMPONENT_MISSING)
	{
		std::fprintf(stderr, "[D3D11] Debug layer not installed, creating device without it.\n");
		flags &= ~D3D11_CREATE_DEVICE_DEBUG;
		hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
			levels, static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION, &m_device, nullptr, &m_context);
	}
#endif
	if (!Check(hr, "D3D11CreateDevice"))
		return false;

#if defined(_DEBUG)
	ComPtr<ID3D11InfoQueue> infoQueue;
	if (SUCCEEDED(m_device.As(&infoQueue)))
	{
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
	}
#endif
	return true;
}

bool Renderer::CreateSwapChain(HWND hwnd)
{
	ComPtr<IDXGIDevice>   dxgiDevice;
	ComPtr<IDXGIAdapter>  adapter;
	ComPtr<IDXGIFactory2> factory;
	if (!Check(m_device.As(&dxgiDevice), "QueryInterface(IDXGIDevice)")) return false;
	if (!Check(dxgiDevice->GetAdapter(&adapter), "GetAdapter")) return false;
	if (!Check(adapter->GetParent(IID_PPV_ARGS(&factory)), "GetParent(IDXGIFactory2)")) return false;

	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Width = m_width;
	desc.Height = m_height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Scaling = DXGI_SCALING_STRETCH;

	if (!Check(factory->CreateSwapChainForHwnd(m_device.Get(), hwnd, &desc, nullptr, nullptr, &m_swapChain),
		"CreateSwapChainForHwnd"))
		return false;

	factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	return true;
}


bool Renderer::CreateBackBufferViews()
{
	ComPtr<ID3D11Texture2D> backBuffer;
	if (!Check(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)), "GetBuffer")) return false;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	if (!Check(m_device->CreateRenderTargetView(backBuffer.Get(), &rtvDesc, &m_rtv), "CreateRenderTargetView"))
		return false;

	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = m_width;
	depthDesc.Height = m_height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ComPtr<ID3D11Texture2D> depth;
	if (!Check(m_device->CreateTexture2D(&depthDesc, nullptr, &depth), "CreateTexture2D(depth)")) return false;
	return Check(m_device->CreateDepthStencilView(depth.Get(), nullptr, &m_dsv), "CreateDepthStencilView");
}

void Renderer::Resize(uint32_t width, uint32_t height)
{
	if (width == 0 || height == 0) return;
	if (width == m_width && height == m_height) return;
	m_width = width;
	m_height = height;

	m_context->OMSetRenderTargets(0, nullptr, nullptr);
	m_rtv.Reset();
	m_dsv.Reset();
	m_context->Flush();

	if (Check(m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0), "ResizeBuffers"))
		CreateBackBufferViews();
}

bool Renderer::CreatePipeline()
{
	const auto shaderPath = std::filesystem::path(SHADER_DIR) / "Basic.hlsl";
	ComPtr<ID3DBlob> vsBlob = CompileShader(shaderPath, "VSMain", "vs_5_0");
	ComPtr<ID3DBlob> psBlob = CompileShader(shaderPath, "PSMain", "ps_5_0");
	if (!vsBlob || !psBlob) return false;

	if (!Check(m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vs),
		"CreateVertexShader"))
		return false;
	if (!Check(m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_ps),
		"CreatePixelShader"))
		return false;

	const D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos),   D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	if (!Check(m_device->CreateInputLayout(layout, static_cast<UINT>(std::size(layout)),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout),
		"CreateInputLayout"))
		return false;
	const Vertex vertices[] = {
		{ { 0.0f,  0.5f, 0.0f },{ 1.0f, 0.0f, 0.0f } }, // 위쪽, 빨강
		{ { 0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f } }, // 오른쪽 아래, 초록
		{ { -0.5f, -0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f } }, // 왼쪽 아래, 파랑
	};
	D3D11_BUFFER_DESC bd{};
	bd.ByteWidth = sizeof(vertices);
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	D3D11_SUBRESOURCE_DATA init{};
	init.pSysMem = vertices;
	return Check(m_device->CreateBuffer(&bd, &init, &m_vertexBuffer), "CreateBuffer(vertex)");
}

void Renderer::Render()
{
	const float clearColor[4] = { 0.02f, 0.02f, 0.03f, 1.0f };
	m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);
	m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ID3D11RenderTargetView* rtvs[] = { m_rtv.Get() };
	m_context->OMSetRenderTargets(1, rtvs, m_dsv.Get());

	const D3D11_VIEWPORT viewport{ 0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height), 0.0f, 1.0f };
	m_context->RSSetViewports(1, &viewport);

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0;
	m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_context->IASetInputLayout(m_inputLayout.Get());
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_context->VSSetShader(m_vs.Get(), nullptr, 0);
	m_context->PSSetShader(m_ps.Get(), nullptr, 0);

	m_context->Draw(3, 0);
}

void Renderer::Present(bool vsync)
{
	m_swapChain->Present(vsync ? 1 : 0, 0);
}

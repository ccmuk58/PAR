#include "Shader.h"
#include "Device.h"
#include "D3DUtil.h"

#include<d3dcompiler.h>
#include<cstdio>

using Microsoft::WRL::ComPtr;
namespace {
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

bool Shader::Init(Device& device, const std::filesystem::path& file, std::span<const D3D11_INPUT_ELEMENT_DESC> layout,
	bool depthOnly)
{
	ID3D11Device* dev = device.Get();
	ComPtr<ID3DBlob> vsBlob = CompileShader(file, "VSMain", "vs_5_0");
	if (!vsBlob) return false;

	if (!Check(dev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vs),
		"CreateVertexShader"))
		return false;

	if (!depthOnly)
	{
		ComPtr<ID3DBlob> psBlob = CompileShader(file, "PSMain", "ps_5_0");
		if (!psBlob) return false;
		if (!Check(dev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_ps),
			"CreatePixelShader"))
			return false;
	}

	return Check(dev->CreateInputLayout(layout.data(), static_cast<UINT>(layout.size()),
		vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout), "CreateInputLayout");
}

void Shader::Bind(ID3D11DeviceContext* ctx) const
{
	ctx->IASetInputLayout(m_inputLayout.Get());
	ctx->VSSetShader(m_vs.Get(), nullptr, 0);
	ctx->PSSetShader(m_ps.Get(), nullptr, 0);
}

#include "Texture.h"

#include "D3DUtil.h"
#include "Device.h"
#include "ImageLoader.h"

using Microsoft::WRL::ComPtr;

bool Texture::Init(Device& device, uint32_t width, uint32_t height, std::span<const uint8_t> pixels, bool srgb)
{
	ID3D11Device* dev = device.Get();
	ID3D11DeviceContext* ctx = device.Context();

	if (width == 0 || height == 0 || pixels.size() != static_cast<size_t>(width) * height * 4)
	{
		std::fprintf(stderr, "Texture init failed: invalid pixel data\n");
		return false;
	}

	// texture 생성 (전체 mip chain)
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	ComPtr<ID3D11Texture2D> texture;
	if (!Check(dev->CreateTexture2D(&desc, nullptr, &texture), "CreateTexture2D"))
		return false;
	ctx->UpdateSubresource(texture.Get(), 0, nullptr, pixels.data(), width * 4, 0);

	// SRV 생성, mipmap 생성
	if (!Check(dev->CreateShaderResourceView(texture.Get(), nullptr, &m_srv), "CreateShaderResourceView"))
		return false;
	ctx->GenerateMips(m_srv.Get());

	return true;
}

bool Texture::Load(Device& device, const std::filesystem::path& file, bool srgb)
{
	ImageData image;
	if (!LoadImageFile(file, image))
		return false;
	return Init(device, image.width, image.height, image.pixels, srgb);
}

void Texture::Bind(ID3D11DeviceContext* ctx, UINT slot) const
{
	ctx->PSSetShaderResources(slot, 1, m_srv.GetAddressOf());
}

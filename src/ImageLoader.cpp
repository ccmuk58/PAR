#include "ImageLoader.h"

#include <windows.h>
#include <objbase.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <cstdio>

using Microsoft::WRL::ComPtr;

namespace
{
	struct ComScope
	{
		ComScope() : m_owned(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {}
		~ComScope() { if (m_owned) CoUninitialize(); }
		ComScope(const ComScope&) = delete;
		ComScope& operator=(const ComScope&) = delete;

	private:
		bool m_owned;
	};

	bool Fail(const std::filesystem::path& file, const char* what, HRESULT hr)
	{
		std::fprintf(stderr, "Image load failed: %ls: %s (0x%08X)\n", file.c_str(), what, static_cast<unsigned>(hr));
		return false;
	}

} // namespace

bool LoadImageFile(const std::filesystem::path& file, ImageData& out)
{
	const ComScope com;

	ComPtr<IWICImagingFactory> factory;
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
	if (FAILED(hr))
		return Fail(file, "CoCreateInstance(WICImagingFactory)", hr);

	ComPtr<IWICBitmapDecoder> decoder;
	hr = factory->CreateDecoderFromFilename(file.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
	if (FAILED(hr))
		return Fail(file, "CreateDecoderFromFilename", hr);

	ComPtr<IWICBitmapFrameDecode> frame;
	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr))
		return Fail(file, "GetFrame", hr);

	// RGBA8 변환
	ComPtr<IWICFormatConverter> converter;
	hr = factory->CreateFormatConverter(&converter);
	if (FAILED(hr))
		return Fail(file, "CreateFormatConverter", hr);
	hr = converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone,
		nullptr, 0.0, WICBitmapPaletteTypeCustom);
	if (FAILED(hr))
		return Fail(file, "IWICFormatConverter::Initialize", hr);

	UINT width = 0;
	UINT height = 0;
	hr = converter->GetSize(&width, &height);
	if (FAILED(hr) || width == 0 || height == 0)
		return Fail(file, "GetSize", hr);

	const UINT stride = width * 4;
	out.width = width;
	out.height = height;
	out.pixels.resize(static_cast<size_t>(stride) * height);
	hr = converter->CopyPixels(nullptr, stride, static_cast<UINT>(out.pixels.size()), out.pixels.data());
	if (FAILED(hr))
		return Fail(file, "CopyPixels", hr);

	return true;
}

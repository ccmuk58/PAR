#pragma once
#include <DirectXMath.h>

struct Vertex
{
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT4 tangent;
    DirectX::XMFLOAT2 uv;
};

#pragma once
#include "Texture.h"
#include <DirectXMath.h>
#include <string>

struct Material
{
    std::string name;

    DirectX::XMFLOAT3 albedo{ 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 specular{ 0.5f, 0.5f, 0.5f };
    float shininess = 64.0f;
	float heightScale = 0.05f;

    Texture albedoMap;
    Texture normalMap;
	Texture heightMap;
};

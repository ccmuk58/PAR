#include "Scene.h"
#include "Device.h"

#include <filesystem>
#include <utility>

bool Scene::Init(Device& device)
{
	const auto assetDir = std::filesystem::path(ASSET_DIR);

	SceneObject teapot;
	if (!teapot.mesh.Load(device, assetDir / "models" / "teapot.obj"))
		return false;
	if (!teapot.albedoMap.Load(device, assetDir / "textures" / "checker.png"))
		return false;
	if (!teapot.normalMap.Load(device, assetDir / "textures" / "checker_normal.png", false))
		return false;
	DirectX::XMStoreFloat4x4(&teapot.world, DirectX::XMMatrixRotationY(-DirectX::XM_PIDIV2));
	m_objects.push_back(std::move(teapot));

	SceneObject ground;
	if (!ground.mesh.Load(device, assetDir / "models" / "ground.obj"))
		return false;
	const uint8_t groundColor[4] = { 150, 150, 150, 255 };
	if (!ground.albedoMap.Init(device, 1, 1, groundColor))
		return false;
	const uint8_t flatNormal[4] = { 128, 128, 255, 255 };
	if (!ground.normalMap.Init(device, 1, 1, flatNormal, false))
		return false;
	m_objects.push_back(std::move(ground));

	return true;
}

void Scene::Update(float /*time*/)
{
}

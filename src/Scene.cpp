#include "Scene.h"
#include "Device.h"

#include <filesystem>
#include <memory>
#include <utility>

bool Scene::Init(Device& device)
{
	const auto assetDir = std::filesystem::path(ASSET_DIR);

	// material
	auto checker = std::make_shared<Material>();
	checker->name = "Checker";
	if (!checker->albedoMap.Load(device, assetDir / "textures" / "checker.png"))
		return false;
	if (!checker->normalMap.Load(device, assetDir / "textures" / "checker_normal.png", false))
		return false;
	m_materials.push_back(checker);

	auto gray = std::make_shared<Material>();
	gray->name = "Ground";
	gray->albedo = { 0.305f, 0.305f, 0.305f };
	m_materials.push_back(gray);

	// object
	SceneObject teapot;
	if (!teapot.mesh.Load(device, assetDir / "models" / "teapot.obj"))
		return false;
	teapot.material = checker;
	DirectX::XMStoreFloat4x4(&teapot.world, DirectX::XMMatrixRotationY(-DirectX::XM_PIDIV2));
	m_objects.push_back(std::move(teapot));

	SceneObject ground;
	if (!ground.mesh.Load(device, assetDir / "models" / "ground.obj"))
		return false;
	ground.material = gray;
	m_objects.push_back(std::move(ground));

	return true;
}

void Scene::Update(float /*time*/)
{
}

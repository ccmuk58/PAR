#include "Scene.h"
#include "Device.h"

#include <filesystem>
#include <memory>
#include <utility>

bool Scene::Init(Device& device)
{
	const auto assetDir = std::filesystem::path(ASSET_DIR);


	// material
	auto gray = std::make_shared<Material>();
	gray->name = "Ground";
	gray->albedo = { 0.305f, 0.305f, 0.305f };
	m_materials.push_back(gray);

	auto wallMaterial = std::make_shared<Material>();
	wallMaterial->name = "Wall";
	m_materials.push_back(wallMaterial);

	// object
	Mesh cube;
	if (!cube.Load(device, assetDir / "models" / "cube.obj"))
		return false;

	SceneObject ground;
	ground.mesh = cube;
	ground.material = gray;
	DirectX::XMStoreFloat4x4(&ground.world,
		DirectX::XMMatrixScaling(15.0f, 0.1f, 15.0f) * DirectX::XMMatrixTranslation(0.0f, -0.1f, 0.0f));
	m_objects.push_back(std::move(ground));

	SceneObject wall;
	wall.mesh = cube;
	wall.material = wallMaterial;
	DirectX::XMStoreFloat4x4(&wall.world,
		DirectX::XMMatrixScaling(2.0f, 1.0f, 0.4f) * DirectX::XMMatrixTranslation(0.0f, 1.0f, 0.0f));
	m_objects.push_back(std::move(wall));

	return true;
}

void Scene::Update(float /*time*/)
{
}

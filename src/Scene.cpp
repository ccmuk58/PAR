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

	auto castleWallMaterialRaw = std::make_shared<Material>();
	castleWallMaterialRaw->name = "Castle Wall";
	if (!castleWallMaterialRaw->albedoMap.Load(device, assetDir / "textures" / "castle_wall_albedo.png"))
		return false;
	m_materials.push_back(castleWallMaterialRaw);

	auto castleWallMaterial_normal = std::make_shared<Material>();
	castleWallMaterial_normal->name = "Castle Wall Normal";
	if (!castleWallMaterial_normal->albedoMap.Load(device, assetDir / "textures" / "castle_wall_albedo.png"))
		return false;
	if (!castleWallMaterial_normal->normalMap.Load(device, assetDir / "textures" / "castle_wall_normal.png"))
		return false;
	m_materials.push_back(castleWallMaterial_normal);

	auto castleWallMaterial_POM = std::make_shared<Material>();
	castleWallMaterial_POM->name = "Castle Wall POM";
	if (!castleWallMaterial_POM->albedoMap.Load(device, assetDir / "textures" / "castle_wall_height.png"))
		return false;
	m_materials.push_back(castleWallMaterial_POM);


	// object
	Mesh cube;
	Mesh sphere;
	if (!cube.Load(device, assetDir / "models" / "cube.obj"))
		return false;
	if (!sphere.Load(device, assetDir / "models" / "sphere.obj"))
		return false;

	SceneObject ground;
	ground.mesh = cube;
	ground.material = gray;
	DirectX::XMStoreFloat4x4(&ground.world,
		DirectX::XMMatrixScaling(15.0f, 0.1f, 15.0f) * DirectX::XMMatrixTranslation(0.0f, -0.1f, 0.0f));
	m_objects.push_back(std::move(ground));

	SceneObject castleWallRaw;
	castleWallRaw.mesh = cube;
	castleWallRaw.material = castleWallMaterialRaw;
	DirectX::XMStoreFloat4x4(&castleWallRaw.world,
		DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(-3.0f, 2.0f, 0.0f));
	m_objects.push_back(std::move(castleWallRaw));

	SceneObject castleWallNormal;
	castleWallNormal.mesh = cube;
	castleWallNormal.material = castleWallMaterial_normal;
	DirectX::XMStoreFloat4x4(&castleWallNormal.world,
		DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(0.0f, 2.0f, 0.0f));
	m_objects.push_back(std::move(castleWallNormal));

	SceneObject castleWallPOM;
	castleWallPOM.mesh = cube;
	castleWallPOM.material = castleWallMaterial_POM;
	DirectX::XMStoreFloat4x4(&castleWallPOM.world,
		DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(3.0f, 2.0f, 0.0f));
	m_objects.push_back(std::move(castleWallPOM));

	return true;
}

void Scene::Update(float /*time*/)
{
}

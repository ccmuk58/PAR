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


	auto castleWallMaterial_POM = std::make_shared<Material>();
	castleWallMaterial_POM->name = "Castle Wall POM";
	castleWallMaterial_POM->heightScale = 0.04f;
	castleWallMaterial_POM->displacementScale = 0.00f;
	if (!castleWallMaterial_POM->albedoMap.Load(device, assetDir / "textures" / "castle_wall_albedo.png"))
		return false;
	if (!castleWallMaterial_POM->normalMap.Load(device, assetDir / "textures" / "castle_wall_normal.png"))
		return false;
	if (!castleWallMaterial_POM->heightMap.Load(device, assetDir / "textures" / "castle_wall_height.png", false))
		return false;
	m_materials.push_back(castleWallMaterial_POM);

	auto castleWallMaterial_Displacement = std::make_shared<Material>();
	castleWallMaterial_Displacement->name = "Castle Wall Displacement";
	castleWallMaterial_Displacement->heightScale = 0.00f;
	castleWallMaterial_Displacement->displacementScale = 0.08f;
	if (!castleWallMaterial_Displacement->albedoMap.Load(device, assetDir / "textures" / "castle_wall_albedo.png"))
		return false;
	if (!castleWallMaterial_Displacement->normalMap.Load(device, assetDir / "textures" / "castle_wall_normal.png"))
		return false;
	if (!castleWallMaterial_Displacement->heightMap.Load(device, assetDir / "textures" / "castle_wall_height.png", false))
		return false;
	m_materials.push_back(castleWallMaterial_Displacement);


	// object
	Mesh cube;
	Mesh cube256;
	Mesh cube_rounded_256;
	Mesh cube_rounded_low;
	if (!cube.Load(device, assetDir / "models" / "cube.obj"))
		return false;
	if (!cube_rounded_low.Load(device, assetDir / "models" / "cube_rounded_low.obj"))
		return false;
	if (!cube256.Load(device, assetDir / "models" / "cube_256.obj"))
		return false;
	if (!cube_rounded_256.Load(device, assetDir / "models" / "cube_rounded_256.obj"))
		return false;

	SceneObject ground;
	ground.mesh = cube;
	ground.material = gray;
	DirectX::XMStoreFloat4x4(&ground.world,
		DirectX::XMMatrixScaling(15.0f, 0.1f, 15.0f) * DirectX::XMMatrixTranslation(0.0f, -0.1f, 0.0f));
	m_objects.push_back(std::move(ground));

	SceneObject castleWallPOM;
	castleWallPOM.mesh = cube_rounded_low;
	castleWallPOM.material = castleWallMaterial_POM;
	DirectX::XMStoreFloat4x4(&castleWallPOM.world,
		DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(-2.5f, 2.0f, 1.0f));
	m_objects.push_back(std::move(castleWallPOM));

	SceneObject castleWallDisplacement;
	castleWallDisplacement.mesh = cube_rounded_256;
	castleWallDisplacement.material = castleWallMaterial_Displacement;
	DirectX::XMStoreFloat4x4(&castleWallDisplacement.world,
		DirectX::XMMatrixScaling(1.0f, 1.0f, 1.0f) * DirectX::XMMatrixTranslation(2.5f, 2.0f, 1.0f));
	m_objects.push_back(std::move(castleWallDisplacement));

	m_pointLight.intensity = 0.0f;
	m_spotLight.intensity = 0.0f;

	return true;
}

void Scene::Update(float /*time*/)
{
}

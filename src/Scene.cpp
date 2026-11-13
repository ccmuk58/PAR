#include "Scene.h"
#include "Device.h"

#include <filesystem>
#include <utility>

using namespace DirectX;

bool Scene::Init(Device& device)
{
	const auto assetDir = std::filesystem::path(ASSET_DIR);

	SceneObject teapot;
	if (!teapot.mesh.Load(device, assetDir / "models" / "teapot.obj"))
		return false;
	if (!teapot.albedoMap.Load(device, assetDir / "textures" / "checker.png"))
		return false;
	m_objects.push_back(std::move(teapot));

	return true;
}

void Scene::Update(float time)
{
	for (SceneObject& object : m_objects)
		XMStoreFloat4x4(&object.world, XMMatrixRotationY(time));
}

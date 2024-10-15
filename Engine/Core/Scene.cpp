#include "Scene.h"

#include "Entity/Entity.h"
#include "Entity/Prefab.h"
#include "Entity/Components/BaseComponents.h"
#include <Common/Debug/Logger.h>
#include <Common/File/Filesystem.h>
#include <Common/Math/ID.h>

namespace Cosmos::Engine
{
	Scene::Scene(std::string name)
		: mName(name)
	{
		mRootPrefab = new Prefab(this, "Root Prefab");
	}

	Scene::~Scene()
	{
		delete mRootPrefab;
	}

	void Scene::OnUpdate(double timestep)
	{
	}

	void Scene::OnRender()
	{
	}

	void Scene::OnEvent(Shared<Platform::EventBase> event)
	{
	}

	void Scene::ClearScene()
	{
		// erase the root's children, this is recursive and erases it's children
		for (auto& child : mRootPrefab->GetChildrenRef()) {
			mRootPrefab->EraseChild(child.second, false);
		}

		// erase all entities the root has
		for (auto& entity : mRootPrefab->GetEntitiesRef()) {
			mRootPrefab->EraseEntity(entity.second, false);
		}

		mRootPrefab->GetChildrenRef().clear();
		mRootPrefab->GetEntitiesRef().clear();
	}

	Datafile Scene::Serialize()
	{
		Datafile scene;
		scene["Name"].SetString(mName);
		
		for (auto& child : mRootPrefab->GetChildrenRef()) {
			Prefab::Serialize(child.second, scene["Hierarchy"]["Prefabs"]);
		}
		
		for (auto& entity : mRootPrefab->GetEntitiesRef()) {
			entity.second->Serialize(scene["Hierarchy"]["Entities"]);
		}
		
		return scene;
	}

	void Scene::Deserialize(Datafile& scene)
	{
		ClearScene();

		// we must re-create prefabs and entities as the file goes
		mName = scene["Name"].GetString();

		Prefab::Deserialize(mRootPrefab, this, scene["Hierarchy"]);
	}
}
#include "Scene.h"

#include "Entity/Entity.h"
#include "Entity/Prefab.h"
#include "Entity/Components/BaseComponents.h"
#include <Common/Debug/Logger.h>
#include <Common/Debug/Profiler.h>
#include <Common/File/Filesystem.h>
#include <Common/Math/ID.h>
#include <Renderer/Core/IMesh.h>

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

	void Scene::OnUpdate(float timestep)
	{
		PROFILER_FUNCTION();

		// meshes update
		auto meshesView = mRegistry.view<IDComponent, TransformComponent, MeshComponent>();
		for(auto entity : meshesView) {
			auto [id, transform, mesh] = meshesView.get<IDComponent, TransformComponent, MeshComponent>(entity);

			if (mesh.mesh == nullptr) {
				continue;
			}

			mesh.mesh->OnUpdate(timestep);
		}
	}

	void Scene::OnRender()
	{
		PROFILER_FUNCTION();

		// meshes rendering
		auto meshesView = mRegistry.view<IDComponent, TransformComponent, MeshComponent>();
		for (auto entity : meshesView) {
			auto [id, transform, mesh] = meshesView.get<IDComponent, TransformComponent, MeshComponent>(entity);

			if (mesh.mesh == nullptr || !mesh.mesh->IsLoaded() || mesh.mesh->IsTransfering()) {
				continue;
			}

			mesh.mesh->OnRender(transform.GetTransform(), id.id->GetValue());
		}
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
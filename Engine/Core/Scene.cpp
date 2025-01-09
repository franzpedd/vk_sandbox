#include "Scene.h"

#include "Entity/Entity.h"
#include "Entity/Prefab.h"
#include "Entity/Components/AllComponents.h"

#include <Common/Debug/Logger.h>
#include <Common/Debug/Profiler.h>
#include <Common/File/Filesystem.h>
#include <Common/Math/ID.h>
#include <Renderer/Core/IContext.h>
#include <Renderer/Core/IMesh.h>
#include <Renderer/Core/ITexture.h>

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

	void Scene::OnRender(uint32_t stage)
	{
		PROFILER_FUNCTION();

		// meshes rendering
		auto meshesView = mRegistry.view<IDComponent, TransformComponent, MeshComponent>();
		for (auto entity : meshesView) {
			auto [id, transform, mesh] = meshesView.get<IDComponent, TransformComponent, MeshComponent>(entity);

			if (mesh.mesh == nullptr || !mesh.mesh->IsLoaded() || mesh.mesh->IsTransfering()) {
				continue;
			}

			mesh.mesh->OnRender(transform.GetTransform(), id.id->GetValue(), (Renderer::IContext::Stage)stage);
		}
	}

	void Scene::OnEvent(Shared<Platform::EventBase> event)
	{
		PROFILER_FUNCTION();
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

	void Scene::FindObjectIntersection(glm::vec3 rayStart, glm::vec3 rayDirection)
	{
		auto meshesView = mRegistry.view<IDComponent, TransformComponent, MeshComponent>();

		for (auto entity : meshesView) {
			auto [id, transform, mesh] = meshesView.get<IDComponent, TransformComponent, MeshComponent>(entity);

			if (mesh.mesh == nullptr || !mesh.mesh->IsLoaded() || mesh.mesh->IsTransfering()) {
				continue;
			}

			Debug_CubeRay(rayStart, rayDirection);
		}
	}

	void Scene::ObjectPicking(const glm::vec3& startPos, const glm::vec3& endPos, const glm::vec3& dir)
	{
		//float closestDistance = FLT_MAX; // Closest intersection distance
		//uint64_t closestID = -1; // ID of the closest AABB
		//
		//auto meshesView = mRegistry.view<IDComponent, TransformComponent, MeshComponent>();
		//for (auto selectable : meshesView) {
		//	auto [id, transform, mesh] = meshesView.get<IDComponent, TransformComponent, MeshComponent>(selectable);
		//
		//	if (mesh.mesh == nullptr || !mesh.mesh->IsLoaded() || mesh.mesh->IsTransfering()) {
		//		continue;
		//	}
		//
		//	float t;
		//	glm::vec3 aabbMin, aabbMax;
		//	glm::vec3 localMin = { -1.0f, -1.0f, -1.0f };
		//	glm::vec3 localMax = { 1.0f, 1.0f, 1.0f };
		//	transform.ComputeAABB(localMin, localMax, transform.GetTransform(), aabbMin, aabbMax);
		//
		//	if (RayAABBCollide(startPos, dir, aabbMin, aabbMax, t)) {
		//
		//		if (t < closestDistance) {
		//			closestDistance = t;
		//			closestID = id.id->GetValue();
		//		}
		//	}
		//}
		//
		// now we must find ID, maybe I should be replacing the main structure behind the Editor's Hierarchy
		// or I could add an selectable component

		// we have an selectable component to set/unset the item as selected. However the only supported component is the mesh component
		//auto meshesView = mRegistry.view<IDComponent, EditorComponent, TransformComponent, MeshComponent>();
		//for (auto selectable : meshesView) {
		//	auto [id, editor, transform, mesh] = meshesView.get<IDComponent, EditorComponent, TransformComponent, MeshComponent>(selectable);
		//
		//	if (mesh.mesh == nullptr) continue;
		//	if (!mesh.mesh->IsLoaded()) continue;
		//	if (mesh.mesh->IsTransfering()) continue;
		//
		//	if (RayAABBCollide(startPos, dir, aabbMin, aabbMax, t)) {
		//
		//	}
		//}
	}

	void Scene::Debug_CubeRay(glm::vec3 startPos, glm::vec3 endPos)
	{
		COSMOS_LOG(Logger::Warn, "This will crash at exit, but it's for debugging at the momment");

		static int line_num = -1;
		line_num++;

		std::string startName = "LineStart";
		startName.append(std::to_string(line_num));
		mRootPrefab->InsertEntity(startName);

		auto it = mRootPrefab->GetEntitiesRef().find(startName);
		if (it != mRootPrefab->GetEntitiesRef().end()) {
			it->second->AddComponent<TransformComponent>();
			it->second->GetComponent<TransformComponent>().translation = startPos;

			it->second->AddComponent<MeshComponent>();
			it->second->GetComponent<MeshComponent>().mesh = Renderer::IMesh::Create();
			it->second->GetComponent<MeshComponent>().mesh->LoadFromFile(GetAssetSubDir("Mesh/cube.gltf"));
		}

		std::string endName = "LineEnd";
		endName.append(std::to_string(line_num));
		mRootPrefab->InsertEntity(endName);

		it = mRootPrefab->GetEntitiesRef().find(endName);
		if (it != mRootPrefab->GetEntitiesRef().end()) {
			it->second->AddComponent<TransformComponent>();
			it->second->GetComponent<TransformComponent>().translation = endPos;
		
			it->second->AddComponent<MeshComponent>();
			it->second->GetComponent<MeshComponent>().mesh = Renderer::IMesh::Create();
			it->second->GetComponent<MeshComponent>().mesh->LoadFromFile(GetAssetSubDir("Mesh/cube.gltf"));
			it->second->GetComponent<MeshComponent>().mesh->SetSelected(true);
		}
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
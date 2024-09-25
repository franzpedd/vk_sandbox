#include "Scene.h"

#include "Entity/Entity.h"
#include "Entity/Components/BaseComponents.h"
#include <Common/Debug/Logger.h>

namespace Cosmos::Engine
{
	Scene::Scene(const Datafile sceneData)
		: mSceneData(sceneData)
	{
	}

	Scene::~Scene()
	{
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

	Entity* Scene::CreateEntity(std::string name)
	{
		entt::entity handle = mRegistry.create();
		Entity* entity = new Entity(this, handle);
		
		// create the id component
		entity->AddComponent<IDComponent>();
		entity->GetComponent<IDComponent>().id = CreateShared<ID>();
		
		// create the name component
		entity->AddComponent<NameComponent>();
		entity->GetComponent<NameComponent>().name = name;
		
		// inserts the entity id into our library of registered entities
		std::string idStr = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
		
		if (mEntities.Exists(idStr.c_str())) {
			COSMOS_LOG(Logger::Error, "The entity creation process has generated an already-in-use id.");
		}
		
		else {
			mEntities.Insert(idStr.c_str(), entity);
		}
		
		// returns a reference to the newly created entity
		return mEntities.GetRef(idStr.c_str());
	}

	void Scene::DestroyEntity(Entity* entity)
	{
		std::string idStr = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());

		if (entity->HasComponent<IDComponent>()) {
			entity->RemoveComponent<IDComponent>();
		}
		
		if (entity->HasComponent<NameComponent>()) {
			entity->RemoveComponent<NameComponent>();
		}
		
		if (entity->HasComponent<TransformComponent>()) {
			entity->RemoveComponent<TransformComponent>();
		}

		mEntities.Erase(idStr.c_str());
		delete entity;
	}

	Entity* Scene::FindEntity(uint64_t id)
	{
		std::string strId = std::to_string(id);

		if (!mEntities.Exists(strId.c_str())) {
			COSMOS_LOG(Logger::Error, "Could not find any entity with id %llu, returning empty", id);
			std::abort();
		}

		return mEntities.GetRef(strId.c_str());
	}
}
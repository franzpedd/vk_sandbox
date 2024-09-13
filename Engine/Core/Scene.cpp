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

	Shared<Entity>& Scene::CreateEntity(std::string name)
	{
		entt::entity handle = mRegistry.create();
		Shared<Entity> entity = CreateShared<Entity>(this, handle);

		// create the id component
		entity->AddComponent<IDComponent>();
		entity->GetComponent<IDComponent>().id = CreateShared<ID>();

		// create the name component
		entity->AddComponent<NameComponent>();
		entity->GetComponent<NameComponent>().name = name;

		// inserts the entity id into our library of registered entities
		std::string idStr = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());

		if (mEntities.Exists(idStr.c_str())) {
			COSMOS_LOG(Logger::Error, "The entity creation process has generated an already-in-use id and will return it's entity instead.");
			entity.reset();
		}

		else {
			mEntities.Insert(idStr.c_str(), entity);
		}

		// returns a reference to the newly created entity
		return mEntities.GetRef(idStr.c_str());
	}

	void Scene::DestroyEntity(Shared<Entity> entity)
	{
		if (entity->HasComponent<IDComponent>()) {
			entity->RemoveComponent<IDComponent>();
		}
		
		if (entity->HasComponent<NameComponent>()) {
			entity->RemoveComponent<NameComponent>();
		}
		
		if (entity->HasComponent<TransformComponent>()) {
			entity->RemoveComponent<TransformComponent>();
		}

		std::string strId = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
		mEntities.Erase(strId.c_str());
	}

	Shared<Entity>& Scene::FindEntity(uint64_t id)
	{
		std::string strId = std::to_string(id);

		if (!mEntities.Exists(strId.c_str())) {
			COSMOS_LOG(Logger::Error, "Could not find any entity with id %llu, returning empty", id);
			std::abort();
		}

		return mEntities.GetRef(strId.c_str());
	}
}
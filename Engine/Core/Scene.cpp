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

	void Scene::Deserialize(Datafile& data)
	{
		// cleanup the current scene
		for (auto& ent : mEntities.GetAllRefs()) {
			DestroyEntity(ent.second);
		}

		mSceneData = data;

		// deserialize all components the entity may have
		for (size_t i = 0; i < data["Entities"].GetChildrenCount(); i++) {
			Datafile entityData = data["Entities"][i];
			Entity* entity = new Entity(this, mRegistry.create());

			IDComponent::Deserialize(entity, entityData);
			NameComponent::Deserialize(entity, entityData);
			TransformComponent::Deserialize(entity, entityData);

			// inserts the entity id into our library of registered entities
			std::string idStr = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
			mEntities.Insert(idStr, entity);
		}
	}

	Datafile Scene::Serealize()
	{
		Datafile save;
		save["Name"].SetString(mSceneData["Name"].GetString());

		for (auto& entity : mEntities.GetAllRefs()) {
			if (entity.second == nullptr) {
				continue;
			}

			IDComponent::Serialize(entity.second, save);
			NameComponent::Serialize(entity.second, save);
			TransformComponent::Serialize(entity.second, save);
		}

		return save;
	}

	Datafile Scene::CreateDefaultScene()
	{
		Datafile save;
		save["Name"].SetString("Default");

		return save;
	}

	bool Scene::IsDefaultScene(Datafile& sceneData)
	{
		Datafile defaultScene = CreateDefaultScene();

		bool result = true;
		result &= defaultScene["Name"].GetString().compare(sceneData["Name"].GetString()) == 0; // has the same name
		result &= defaultScene.GetChildrenCount() == sceneData.GetChildrenCount(); // has the same number of properties
		
		// we're not checking all possibilities because it's a development stage
		// however this is pretty much enought to qualify the scene as the default one because Default.scene is supposed to be unique
		return result;
	}
}
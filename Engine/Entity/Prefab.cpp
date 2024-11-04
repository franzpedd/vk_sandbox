#include "Prefab.h"

#include "Entity.h"
#include "Components/BaseComponents.h"
#include "Core/Scene.h"
#include <Common/Debug/Logger.h>

namespace Cosmos::Engine
{
    Prefab::Prefab(Scene* scene, std::string name)
        : mScene(scene), mName(name)
    {
        mID = CreateUnique<ID>();
    }

    Prefab::Prefab(Scene* scene, uint64_t id, std::string name)
        : mScene(scene), mName(name)
    {
        mID = CreateUnique<ID>(id);
    }

    Prefab::~Prefab()
    {

    }

    void Prefab::InsertChild(std::string name)
    {
        mChildren.insert({ "New Prefab", new Prefab(mScene, name) });
    }

    void Prefab::EraseChild(Prefab* prefab, bool eraseFromMultimap)
    {
        // remove the prefab from the list of prefabs
        bool found = false;
        auto range = mChildren.equal_range(prefab->GetNameRef());
        for (auto& it = range.first; it != range.second; ++it) {
            if (it->second == prefab) {
                if (eraseFromMultimap) {
                    mChildren.erase(it);
                }
                found = true;
                break;
            }
        }
        
        if (!found) {
            COSMOS_LOG(Logger::Error, "Could not find the given entity to destroy it");
            return;
        }

        // delete prefab and it's children
        Recursively_Delete(prefab);
    }

    void Prefab::InsertEntity(std::string name)
    {
        entt::entity handle = mScene->GetEntityRegistryRef().create();
        Entity* entity = new Entity(mScene, handle);

        // create identifiers
        entity->AddComponent<IDComponent>();
        entity->AddComponent<NameComponent>(name);

        mEntities.insert({ name, entity });
    }

    void Prefab::EraseEntity(Entity* entity, bool eraseFromMultimap)
    {
        bool found = false;
        auto range = mEntities.equal_range(entity->GetComponent<NameComponent>().name);
        for (auto& it = range.first; it != range.second; ++it) {
            if (it->second == entity) {
                if (eraseFromMultimap) {
                    mEntities.erase(it);
                }
                found = true;
                break;
            }
        }
        
        if (!found) {
            COSMOS_LOG(Logger::Error, "Could not find the given entity to destroy it");
            return;
        }

        entity->RemoveComponent<NameComponent>();
        entity->RemoveComponent<IDComponent>();
        entity->RemoveComponent<MeshComponent>();
        
        delete entity;
    }

    void Prefab::Serialize(Prefab* prefab, Datafile& sceneData)
    {
        std::string id = "Prefab:";
        id.append(std::to_string(prefab->GetIDValue()));

        sceneData[id]["Name"].SetString(prefab->GetNameRef());
        sceneData[id]["Id"].SetString(std::to_string(prefab->GetIDValue()));
        
        for (auto& child : prefab->GetChildrenRef()) {
            child.second->Serialize(child.second, sceneData[id]["Prefabs"]);
        }
        
        for (auto& entity : prefab->GetEntitiesRef()) {
            entity.second->Serialize(sceneData[id]["Entities"]);
        }
    }

    void Prefab::Deserialize(Prefab* prefab, Scene* scene, Datafile& sceneData)
    {
        if(sceneData.Exists("Prefabs")) {
            for(size_t i = 0; i < sceneData["Prefabs"].GetChildrenCount(); i++) {
                Datafile prefabData = sceneData["Prefabs"][i];
                std::string name = prefabData["Name"].GetString();
                std::string id = prefabData["Id"].GetString();

                Prefab* child = new Prefab(scene, std::stoull(id), name);
                prefab->GetChildrenRef().insert({ name, child });

                Deserialize(child, scene, prefabData);
            }
        }
        
        if(sceneData.Exists("Entities")) {
            for (size_t i = 0; i < sceneData["Entities"].GetChildrenCount(); i++) {
                Datafile data = sceneData["Entities"][i];
                entt::entity handle = scene->GetEntityRegistryRef().create();
                Entity* entity = new Entity(scene, handle);

                IDComponent::Deserialize(entity, data);
                NameComponent::Deserialize(entity, data);
                TransformComponent::Deserialize(entity, data);

                prefab->GetEntitiesRef().insert({ entity->GetComponent<NameComponent>().name, entity});
            }
        }
    }

    void Prefab::Recursively_Delete(Prefab* current)
    {
        for (auto& entity : current->GetEntitiesRef()) {
            current->EraseEntity(entity.second, false);
        }

        for (auto& child : current->GetChildrenRef()) {
            Recursively_Delete(child.second);
        }

        current->GetEntitiesRef().clear();
        current->GetChildrenRef().clear();

        delete current;
    }
}
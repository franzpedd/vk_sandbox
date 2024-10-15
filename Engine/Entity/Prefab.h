#pragma once

#include <Common/File/Datafile.h>
#include <Common/Math/ID.h>
#include <Common/Util/Memory.h>
#include <string>
#include <map>

// forward declarations
namespace Cosmos::Engine { class Entity; }
namespace Cosmos::Engine { class Scene; }

namespace Cosmos::Engine
{
    class Prefab
    {
    public:

        // constructor
        Prefab(Scene* scene, std::string name = "New Prefab");

        // constructor
        Prefab(Scene* scene, uint64_t id, std::string name = "New Prefab");

        // destructor
        ~Prefab();

        // returns the prefab's name
        inline std::string& GetNameRef() { return mName; }

        // returns this prefab's id value
        inline uint64_t GetIDValue() const { return mID->GetValue(); }

        // returns a reference to the children of this prefab
        inline std::multimap<std::string, Prefab*>& GetChildrenRef() { return mChildren; }

        // returns a reference to the entities of this prefab
        inline std::multimap<std::string, Engine::Entity*>& GetEntitiesRef() { return mEntities; }

    public:

        // adds a child of this prefab, a sub-prefab
        void InsertChild(std::string name);

        // removes and destroys a child prefab, and it's children-tree
        void EraseChild(Prefab* prefab, bool eraseFromMultimap = true);

        // adds an entity to this prefab
        void InsertEntity(std::string name);

        // removes and destroys a given entity
        void EraseEntity(Entity* entity, bool eraseFromMultimap = true);

    public:

        // saves the prefab entities and sub-prefabs into the datafile
        static void Serialize(Prefab* prefab, Datafile& scene);

        // loads the prefab entities and sub-prefabs given the datafile
        static void Deserialize(Prefab* prefab, Scene* scene, Datafile& sceneData);

    private:

        // recursively erases and free 
        static void Recursively_Delete(Prefab* current);

    private:

        Scene* mScene = nullptr;
        Unique<ID> mID;
        std::string mName;
        std::multimap<std::string, Prefab*> mChildren = {};
        std::multimap<std::string, Engine::Entity*> mEntities = {};
    };
}
#pragma once

#include "Wrapper/Entt.h"
#include <Common/File/Datafile.h>
#include <Common/Math/ID.h>
#include <Common/Util/Library.h>
#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Engine { class Entity; }
namespace Cosmos::Platform { class EventBase; }

namespace Cosmos::Engine
{
	class Scene
	{
	public:

		// constructor
		Scene(const Datafile sceneData);

		// destructor
		~Scene();

		// returns a reference to the entt registry
		inline entt::registry& GetEntityRegistryRef() { return mRegistry; }

		// returns a reference to the library of entities 
		inline Library<Shared<Entity>>& GetEntityLibraryRef() { return mEntities; }

	public:

		// returns the scene's name
		inline std::string Getname() { return mSceneData["Name"].GetString(); }

		// sets the scene's name
		inline void SetName(std::string name) { mSceneData["Name"].SetString(name); }

	public:

		// updates the scene logic
		void OnUpdate(double timestep);

		// draws the scene objects
		void OnRender();

		// called when an event happens
		void OnEvent(Shared<Platform::EventBase> event);

	public:

		// creates and returns an entity
		Shared<Entity>& CreateEntity(std::string name);

		// destroys an entity
		void DestroyEntity(Shared<Entity> entity);

		// finds an entity by it's unique identifier number
		Shared<Entity>& FindEntity(uint64_t id);

	private:

		Datafile mSceneData;
		entt::registry mRegistry;
		Library<Shared<Entity>> mEntities;
	};
}
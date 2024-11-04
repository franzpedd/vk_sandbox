#pragma once

#include "Wrapper/Entt.h"
#include <Common/File/Datafile.h>
#include <Common/Util/Library.h>
#include <Common/Util/Memory.h>
#include <string>

// forward declarations
namespace Cosmos::Engine { class Entity; }
namespace Cosmos::Engine { class Prefab; }
namespace Cosmos::Platform { class EventBase; }

namespace Cosmos::Engine
{
	class Scene
	{
	public:

		// constructor
		Scene(std::string name = "Empty Scene");

		// destructor
		~Scene();

		// returns a reference to the entt registry
		inline entt::registry& GetEntityRegistryRef() { return mRegistry; }

		// returns the root prefab of the scene
		inline Prefab* GetRootPrefab() { return mRootPrefab; }

	public:

		// returns the scene's name
		inline std::string GetName() const { return mName; }

		// sets the scene's name
		inline void SetName(const std::string& name) { mName = name; }

	public:

		// updates the scene logic
		void OnUpdate(float timestep);

		// draws the scene objects
		void OnRender();

		// called when an event happens
		void OnEvent(Shared<Platform::EventBase> event);

	public:

		// erases all contents the scene has
		void ClearScene();

	public:

		// saves the scene on disk, it gets saved on the scene's folder with "mName.scene" as it's name
		Datafile Serialize();

		// reads the scene from disk
		void Deserialize(Datafile& scene);

	private:

		entt::registry mRegistry;
		std::string mName;
		Prefab* mRootPrefab;
	};
}
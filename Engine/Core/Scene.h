#pragma once

#include "Wrapper/Entt.h"
#include <Common/Math/Math.h>
#include <Common/File/Datafile.h>
#include <Common/Util/Library.h>
#include <Common/Util/Memory.h>
#include <string>
#include <vector>

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

		// draws the scene objects, pipeline must match Renderer::IContext::Stage
		void OnRender(uint32_t stage);

		// called when an event happens
		void OnEvent(Shared<Platform::EventBase> event);

	public:

		// erases all contents the scene has
		void ClearScene();

		// attempts to find an entity that may intersect with a given ray
		void FindObjectIntersection(glm::vec3 rayStart, glm::vec3 rayDirection);

		// attempts to select an object checking collision with a ray between two points
		void ObjectPicking(const glm::vec3& startPos, const glm::vec3& endPos, const glm::vec3& dir);

		// creates 2 entities, representing a line
		void Debug_CubeRay(glm::vec3 startPos, glm::vec3 endPos);

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
#pragma once

#include "Core/Scene.h"
#include "Wrapper/Entt.h"
#include <Common/Math/ID.h>
#include <Common/File/Datafile.h>

namespace Cosmos::Engine
{
	class Entity
	{
	public:

		// constructor
		Entity(Scene* scene, entt::entity handle);

		// destructor
		~Entity() = default;

		// returns the entity handle
		inline entt::entity GetHandle() const { return mHandle; }

	public:

		// checks if entity has a certain component
		template<typename T>
		bool HasComponent()
		{
			return mScene->GetEntityRegistryRef().all_of<T>(mHandle);
		}

		// returns the component
		template<typename T>
		T& GetComponent()
		{
			return mScene->GetEntityRegistryRef().get<T>(mHandle);
		}

		// adds a component for the entity
		template<typename T, typename...Args>
		T& AddComponent(Args&&... args)
		{
			return mScene->GetEntityRegistryRef().emplace_or_replace<T>(mHandle, std::forward<Args>(args)...);
		}

		// removes the component
		template<typename T>
		void RemoveComponent()
		{
			if (HasComponent<T>()) {
				mScene->GetEntityRegistryRef().remove<T>(mHandle);
			}
		}

	public:

		// saves the scene's components into the datafile
		void Serialize(Datafile& datafile);

	private:

		Scene* mScene;
		entt::entity mHandle;
	};
}
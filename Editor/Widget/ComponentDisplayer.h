#pragma once

// forward declaration
namespace Cosmos::Engine { class Entity; }
namespace Cosmos::Editor { class PrefabHierarchy; }

namespace Cosmos::Editor
{
	class ComponentDisplayer
	{
	public:

		// constructor
		ComponentDisplayer(PrefabHierarchy* prefabHierarchy);

		// destructor
		~ComponentDisplayer();

	public:

		// called to update the window, we're also passing the last selected entity
		void OnUpdate();

	private:

		// displays all components an entity has
		void UpdateEntityComponents(Engine::Entity* entity);

		// adds a menu option with the component name
		template<typename T>
		void AddComponentOnList(const char* name, Engine::Entity* entity);

		// displays the component configuration
		template<typename T, typename F>
		void ShowComponent(const char* name, Engine::Entity* entity, F function);

	private:

		PrefabHierarchy* mPrefabHierarchy;
	};
}
#pragma once

// forward declaration
namespace Cosmos::Engine { class Entity; }

namespace Cosmos::Editor
{
	class ComponentDisplayer
	{
	public:

		// constructor
		ComponentDisplayer();

		// destructor
		~ComponentDisplayer();

	public:

		// called to update the window, we're also passing the last selected entity
		void OnUpdate(Engine::Entity* entity);

	private:

		// displays all components an entity has
		void UpdateEntityComponents(Engine::Entity* entity);

		// adds a menu option with the component name
		template<typename T>
		void AddComponentOnList(const char* name, Engine::Entity* entity);

		template<typename T, typename F>
		void UpdateComponent(const char* name, Engine::Entity* entity, F function);
	};
}
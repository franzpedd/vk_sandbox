#pragma once

#include <Renderer/GUI/Widget.h>
#include <map>
#include <string>
#include <vector>

// forward declarations
namespace Cosmos::Engine { class Entity; }
namespace Cosmos::Engine { class Prefab; }
namespace Cosmos::Editor { class Application; }
namespace Cosmos::Editor { class ComponentDisplayer; }
namespace Cosmos::Editor { class Explorer; }

namespace Cosmos::Editor
{
	class PrefabHierarchy : public Renderer::Widget
	{
	public:

		// constructor
		PrefabHierarchy(Application* application, Explorer* explorer);

		// destructor
		virtual ~PrefabHierarchy();

	public:

		// returns the last selected entity pointer
		inline Engine::Entity* GetSelectedEntity() { return mLastSelectedEntity; }

	public:

		// updates the tick logic
		virtual void OnUpdate() override;

	public:

		// sets on/off the debug window
		inline void SetOpened(bool value) { mOpened = value; }

		// returns if the debug window is opened
		inline bool IsOpened() { return mOpened; }

	private:

		// updates the display node for the prefabs
		void UpdatePrefabs(Engine::Prefab* parent, Engine::Prefab* current);

		// updates the display node for the entity 
		void UpdateEntity(Engine::Prefab* current, Engine::Entity* entity);

		// deletes all prefabs and entities requested to be deleted
		void UpdateDeletionQueue();

		// draws the menubar and handle it's options/popups
		void DisplayMenubar();

		// draws the root right-click menu options
		void DisplayRootMenu();

		// draws the prefabs right-click menu options
		void DisplayPrefabMenu(Engine::Prefab* parent, Engine::Prefab* current);

		// draws the entity right-click menu options
		void DisplayEntityMenu(Engine::Prefab* current, Engine::Entity* entity);

		// makes a place to drag entities or prefabs from
		void DragAndDropSource(bool isEntity, Engine::Entity* entity, Engine::Prefab* parent, Engine::Prefab* current);

		// makes a place to drop entities and prefabs
		void DragAndDropTarget(Engine::Prefab* movingTo);

	private:

		Application* mApplication = nullptr;
		Explorer* mExplorer = nullptr;
		bool mOpened = true;
		Unique<ComponentDisplayer> mComponentDisplayer;

		struct EntityRequest
		{
			Engine::Prefab* current;
			Engine::Entity* entity;

			// constructor
			EntityRequest(Engine::Prefab* current = nullptr, Engine::Entity* entity = nullptr) : current(current), entity(entity) {}
		};

		struct PrefabRequest
		{
			Engine::Prefab* parent;
			Engine::Prefab* current;

			// constructor
			PrefabRequest(Engine::Prefab* parent = nullptr, Engine::Prefab* current = nullptr) : parent(parent), current(current) {}

			// the prefab we are moving cannot be moved into any subprefab of the prefab itself
			static void IsValidMove(Engine::Prefab* movingTo, Engine::Prefab* current, bool* found);
		};

		EntityRequest mMovingEntity;
		PrefabRequest mMovingPrefab;
		std::vector<PrefabRequest> mPrefabDeletionQueue = {};
		std::vector<EntityRequest> mEntityDeletionQueue = {};
		
		// should become vectors when multi-selection are enabled
		Engine::Prefab* mRenamingPrefab = nullptr;
		Engine::Entity* mRenamingEntity = nullptr;
		Engine::Entity* mLastSelectedEntity = nullptr;	
	};
}
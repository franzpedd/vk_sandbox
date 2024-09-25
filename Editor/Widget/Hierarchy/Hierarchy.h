#pragma once

#include <Renderer/GUI/Widget.h>
#include <map>
#include <string>
#include <vector>

// forward declarations
namespace Cosmos::Engine { class Entity; }
namespace Cosmos::Editor { class Application; }
namespace Cosmos::Editor { class ComponentDisplayer; }

namespace Cosmos::Editor
{
	class Hierarchy : public Renderer::Widget
	{
	public:

		struct Group
		{
			std::string name = "New Group";
			std::multimap<std::string, Group*> subgroups = {};
			std::multimap<std::string, Engine::Entity*> entities = {};
		};

		struct GroupDeletionRequest
		{
			Group* parent;
			Group* current;

			// constructor
			GroupDeletionRequest(Group* parent, Group* current) : parent(parent), current(current) {}
		};

		struct EntityDeletionRequest
		{
			Group* current;
			Engine::Entity* entity;

			// constructor
			EntityDeletionRequest(Group* current, Engine::Entity* entity) : current(current), entity(entity) {}
		};

		struct EntityMovingRequest
		{
			Group* current;
			Engine::Entity* entity;
		};

		struct GroupMovingRequest
		{
			Group* parent;
			Group* current;

			// the group we are moving cannot be moved into any subgroup of the group itself
			void IsValidMove(Group* movingTo, Group* current, bool* found);
		};

	public:

		// constructor
		Hierarchy(Application* application);

		// destructor
		virtual ~Hierarchy();

	public:

		// updates the tick logic
		virtual void OnUpdate() override;

	public:

		// sets on/off the debug window
		inline void SetOpened(bool value) { mOpened = value; }

		// returns if the debug window is opened
		inline bool IsOpened() { return mOpened; }

	private:

		// updates the display node for the groups
		void UpdateGroups(Group* parent, Group* current);

		// updates the display node for the entity 
		void UpdateEntity(Group* current, Engine::Entity* entity);

		// deletes all groups and entities requested to be deleted
		void UpdateDeletionQueue();

		// recursively destroy group hierarchy
		void DestroyGroupHierarchy(Group* parent, Group* current);

		// draws the root right-click menu options
		void DisplayRootMenu();

		// draws the group right-click menu options
		void DisplayGroupMenu(Group* parent, Group* current);

		// draws the entity right-click menu options
		void DisplayEntityMenu(Group* current, Engine::Entity* entity);

		// makes a place to drag entities or groups from
		void DragAndDropSource(int32_t movingType, Engine::Entity* entity, Group* parent, Group* current);

		// makes a place to drop entities and groups
		void DragAndDropTarget(Group* movingTo);

	private:

		Application* mApplication = nullptr;
		bool mOpened = true;
		Unique<ComponentDisplayer> mComponentDisplayer;
		Group* mRoot = nullptr;
		Group* mRenamingGroup = nullptr;
		Engine::Entity* mRenamingEntity = nullptr;
		Engine::Entity* mLastSelectedEntity = nullptr;
		EntityMovingRequest* mMovingEntity = nullptr;
		GroupMovingRequest* mMovingGroup = nullptr;
		std::vector<GroupDeletionRequest> mGroupDeletionQueue = {};
		std::vector<EntityDeletionRequest> mEntityDeletionQueue = {};
	};
}
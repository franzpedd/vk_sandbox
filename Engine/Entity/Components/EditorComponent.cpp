#include "EditorComponent.h"

#include "Entity/Entity.h"
#include "IDComponent.h"

namespace Cosmos::Engine
{
	EditorComponent::EditorComponent(bool selectable)
		: selectable(selectable)
	{
	}

	void EditorComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if (entity->HasComponent<EditorComponent>()) {
			std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
			dataFile[uuid]["Editor"]["Selectable"].SetInt(entity->GetComponent<EditorComponent>().selectable);
		}
	}

	void EditorComponent::Deserialize(Entity* entity, Datafile& dataFile)
	{
		if (dataFile.Exists("Editor")) {
			int32_t selectable = dataFile["Editor"]["Selectable"].GetInt();
			entity->AddComponent<EditorComponent>((bool)selectable);
		}
	}
}
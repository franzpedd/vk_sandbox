#include "NameComponent.h"

#include "Entity/Entity.h"
#include "IDComponent.h"

namespace Cosmos::Engine
{
	void NameComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if (entity->HasComponent<NameComponent>()) {
			std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
			dataFile[uuid]["Name"].SetString(entity->GetComponent<NameComponent>().name);
		}
	}

	void NameComponent::Deserialize(Entity* entity, Datafile& dataFile)
	{
		if (dataFile.Exists("Name")) {
			std::string name = dataFile["Name"].GetString();
			entity->AddComponent<NameComponent>(name);
		}
	}
}
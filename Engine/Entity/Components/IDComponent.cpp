#include "IDComponent.h"

#include "Entity/Entity.h"

namespace Cosmos::Engine
{
	IDComponent::IDComponent()
	{
		id = CreateUnique<ID>();
		uint64_t encoded = id->GetValue();
		std::memcpy(&encoded_id, &encoded, sizeof(glm::vec2));
	}

	IDComponent::IDComponent(uint64_t initialID)
	{
		id = CreateUnique<ID>(initialID);
		uint64_t encoded = id->GetValue();
		std::memcpy(&encoded_id, &encoded, sizeof(glm::vec2));
	}

	void IDComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if (entity->HasComponent<IDComponent>()) {
			std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
			dataFile[uuid]["ID"].SetString(uuid);
		}
	}

	void IDComponent::Deserialize(Entity* entity, Datafile& dataFile)
	{
		if (dataFile.Exists("ID")) {
			std::string id = dataFile["ID"].GetString();
			entity->AddComponent<IDComponent>();
			entity->GetComponent<IDComponent>().id = CreateUnique<Cosmos::ID>(std::stoull(id, 0, 10));
		}
	}
}
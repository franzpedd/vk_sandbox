#include "BaseComponents.h"

#include "Entity/Entity.h"

namespace Cosmos::Engine
{
	void IDComponent::Serialize(Shared<Entity> entity, Datafile& dataFile)
	{
		std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
		dataFile["Entities"][uuid]["ID"].SetString(uuid);
	}

	void IDComponent::Deserialize(Shared<Entity> entity, Datafile& dataFile)
	{
		std::string id = dataFile["ID"].GetString();
		entity->AddComponent<IDComponent>();
		entity->GetComponent<IDComponent>().id = CreateUnique<Cosmos::ID>(std::stoull(id, 0, 10));
	}

	void NameComponent::Serialize(Shared<Entity> entity, Datafile& dataFile)
	{
		std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
		dataFile["Entities"][uuid]["Name"].SetString(entity->GetComponent<NameComponent>().name);
	}

	void NameComponent::Deserialize(Shared<Entity> entity, Datafile& dataFile)
	{
		std::string name = dataFile["Name"].GetString();
		entity->AddComponent<NameComponent>(name);
	}

	void TransformComponent::Serialize(Shared<Entity> entity, Datafile& dataFile)
	{
		if (entity->HasComponent<TransformComponent>()) {
			std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
			auto& component = entity->GetComponent<TransformComponent>();
			auto& place = dataFile["Entities"][uuid]["Transform"];

			place["Translation"]["X"].SetDouble(component.translation.x);
			place["Translation"]["Y"].SetDouble(component.translation.y);
			place["Translation"]["Z"].SetDouble(component.translation.z);

			place["Rotation"]["X"].SetDouble(component.rotation.x);
			place["Rotation"]["Y"].SetDouble(component.rotation.y);
			place["Rotation"]["Z"].SetDouble(component.rotation.z);

			place["Scale"]["X"].SetDouble(component.scale.x);
			place["Scale"]["Y"].SetDouble(component.scale.y);
			place["Scale"]["Z"].SetDouble(component.scale.z);
		}
	}

	void TransformComponent::Deserialize(Shared<Entity> entity, Datafile& dataFile)
	{
		// check if transform exists
		if (dataFile.Exists("Transform")) {
			entity->AddComponent<TransformComponent>();
			auto& component = entity->GetComponent<TransformComponent>();
		
			// read translation
			auto& dataT = dataFile["Transform"]["Translation"];
			component.translation = { dataT["X"].GetDouble(), dataT["Y"].GetDouble(), dataT["Z"].GetDouble() };
		
			// read rotation
			auto& dataR = dataFile["Transform"]["Rotation"];
			component.rotation = { dataR["X"].GetDouble(), dataR["Y"].GetDouble(), dataR["Z"].GetDouble() };
		
			// read scale
			auto& dataS = dataFile["Transform"]["Scale"];
			component.scale = { dataS["X"].GetDouble(), dataS["Y"].GetDouble(), dataS["Z"].GetDouble() };
		}
	}
}
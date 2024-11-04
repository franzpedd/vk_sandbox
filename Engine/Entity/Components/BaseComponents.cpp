#include "BaseComponents.h"

#include "Entity/Entity.h"

namespace Cosmos::Engine
{
	IDComponent::IDComponent()
	{
		id = CreateUnique<ID>();
	}

	IDComponent::IDComponent(uint64_t customid)
	{
		id = CreateUnique<ID>(customid);
	}

	void IDComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if(entity->HasComponent<IDComponent>()) {
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

	NameComponent::NameComponent(std::string name)
		: name(name)
	{
	}

	void NameComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if(entity->HasComponent<NameComponent>()) {
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

	TransformComponent::TransformComponent(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale)
		: translation(translation), rotation(rotation), scale(scale)
	{
	}

	void TransformComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
		if (entity->HasComponent<TransformComponent>()) {
			std::string uuid = std::to_string(entity->GetComponent<IDComponent>().id->GetValue());
			auto& component = entity->GetComponent<TransformComponent>();
			auto& place = dataFile[uuid]["Transform"];

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

	void TransformComponent::Deserialize(Entity* entity, Datafile& dataFile)
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

	glm::mat4 TransformComponent::GetTransform() const
	{
		glm::mat4 rot = glm::toMat4(glm::quat(rotation));
		return glm::translate(glm::mat4(1.0f), translation) * rot * glm::scale(glm::mat4(1.0f), scale);
	}

	glm::mat4 TransformComponent::GetNormal() const
	{
		return glm::transpose(glm::inverse(glm::mat3(GetTransform())));
	}

	MeshComponent::MeshComponent()
	{
	}

	void MeshComponent::Serialize(Entity* entity, Datafile& dataFile)
	{
	}

	void MeshComponent::Deserialize(Entity* entity, Datafile& dataFile)
	{
	}
}
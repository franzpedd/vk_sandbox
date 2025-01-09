#include "TransformComponent.h"

#include "Entity/Entity.h"
#include "IDComponent.h"

namespace Cosmos::Engine
{
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
		if (dataFile.Exists("Transform")) {
			entity->AddComponent<TransformComponent>();
			auto& component = entity->GetComponent<TransformComponent>();

			auto& dataT = dataFile["Transform"]["Translation"];
			component.translation = { dataT["X"].GetDouble(), dataT["Y"].GetDouble(), dataT["Z"].GetDouble() };

			auto& dataR = dataFile["Transform"]["Rotation"];
			component.rotation = { dataR["X"].GetDouble(), dataR["Y"].GetDouble(), dataR["Z"].GetDouble() };

			auto& dataS = dataFile["Transform"]["Scale"];
			component.scale = { dataS["X"].GetDouble(), dataS["Y"].GetDouble(), dataS["Z"].GetDouble() };
		}
	}

	const glm::mat4 TransformComponent::GetTransform()
	{
		glm::mat4 rot = glm::toMat4(glm::quat(rotation));
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), translation) * rot * glm::scale(glm::mat4(1.0f), scale);
		return modelMatrix;
	}

	void TransformComponent::ComputeAABB(const glm::vec3& localMin, const glm::vec3& localMax, const glm::mat4& modelMatrix, glm::vec3& aabbMin, glm::vec3& aabbMax)
	{
		// define the 8 corners of the local bounding box
		std::array<glm::vec3, 8> corners = {
			glm::vec3(localMin.x, localMin.y, localMin.z),
			glm::vec3(localMax.x, localMin.y, localMin.z),
			glm::vec3(localMin.x, localMax.y, localMin.z),
			glm::vec3(localMax.x, localMax.y, localMin.z),
			glm::vec3(localMin.x, localMin.y, localMax.z),
			glm::vec3(localMax.x, localMin.y, localMax.z),
			glm::vec3(localMin.x, localMax.y, localMax.z),
			glm::vec3(localMax.x, localMax.y, localMax.z)
		};

		// initialize the AABB min and max to extreme values
		aabbMin = glm::vec3(std::numeric_limits<float>::max());
		aabbMax = glm::vec3(std::numeric_limits<float>::lowest());

		// transform each corner and update the AABB
		for (const auto& corner : corners) {
			glm::vec4 transformedCorner = modelMatrix * glm::vec4(corner, 1.0f); // transform corner
			aabbMin = glm::min(aabbMin, glm::vec3(transformedCorner)); // update min
			aabbMax = glm::max(aabbMax, glm::vec3(transformedCorner)); // update max
		}
	}
}
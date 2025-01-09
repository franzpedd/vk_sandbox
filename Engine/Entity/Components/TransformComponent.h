#pragma once

#include <Common/File/Datafile.h>
#include <Common/Math/Math.h>

// forward declarations
namespace Cosmos::Engine { class Entity; }

namespace Cosmos::Engine
{
	struct TransformComponent
	{
		glm::vec3 translation;
		glm::vec3 rotation;
		glm::vec3 scale;

		// constructor
		TransformComponent(glm::vec3 translation = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);

	public:

		// returns the transformation matrix
		const glm::mat4 GetTransform();

		// calculates the object's axis-aligned box boundaries
		void ComputeAABB(const glm::vec3& localMin, const glm::vec3& localMax, const glm::mat4& modelMatrix, glm::vec3& aabbMin, glm::vec3& aabbMax);
	};
}
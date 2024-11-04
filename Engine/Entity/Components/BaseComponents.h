#pragma once

#include <Common/File/Datafile.h>
#include <Common/Math/ID.h>
#include <Common/Math/Math.h>
#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Engine { class Entity; }
namespace Cosmos::Renderer { class IMesh; }

namespace Cosmos::Engine
{
	struct IDComponent
	{
		Unique<ID> id;

		// constructor
		IDComponent();

		// constructor
		IDComponent(uint64_t customid);

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);
	};

	struct NameComponent
	{
		std::string name;

		// constructor
		NameComponent(std::string name);

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);
	};

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
		glm::mat4 GetTransform() const;

		// returns the normal matrix
		glm::mat4 GetNormal() const;
	};

	struct MeshComponent
	{
		Shared<Renderer::IMesh> mesh;

		// constructor
		MeshComponent();

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);
	};
}
#pragma once

#include <Common/File/Datafile.h>
#include <Common/Math/ID.h>
#include <Common/Math/Math.h>
#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Engine { class Entity; }

namespace Cosmos::Engine
{
	struct IDComponent
	{
		Unique<ID> id;
		glm::vec2 encoded_id;

		// constructor
		IDComponent();

		// constructor
		IDComponent(uint64_t initialID);

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);
	};
}
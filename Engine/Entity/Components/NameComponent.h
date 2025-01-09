#pragma once

#include <Common/File/Datafile.h>
#include <string>

// forward declarations
namespace Cosmos::Engine { class Entity; }

namespace Cosmos::Engine
{
	struct NameComponent
	{
		std::string name;

		// constructor
		NameComponent(std::string name) : name(name) {};

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);
	};
}
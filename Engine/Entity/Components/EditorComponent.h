#pragma once

#include <Common/File/Datafile.h>

// forward declarations
namespace Cosmos::Engine { class Entity; }

namespace Cosmos::Engine
{
	struct EditorComponent
	{
		bool selectable = true;

		// constructor
		EditorComponent(bool selectable = true);

		// destructor
		~EditorComponent() = default;

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);
	};
}
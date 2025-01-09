#pragma once

#include <Common/File/Datafile.h>
#include <Common/Math/ID.h>
#include <Common/Util/Memory.h>

// forward declarations
namespace Cosmos::Engine { class Entity; }
namespace Cosmos::Renderer { class IMesh; }

namespace Cosmos::Engine
{
	struct MeshComponent
	{
		Shared<Renderer::IMesh> mesh;

		// constructor
		MeshComponent() = default;

		// saves the component into a data file
		static void Serialize(Entity* entity, Datafile& dataFile);

		// loads the component into the entity from data file
		static void Deserialize(Entity* entity, Datafile& dataFile);
	};
}
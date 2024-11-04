#include "IMesh.h"

#include "Vulkan/Mesh.h"

namespace Cosmos::Renderer
{
	Shared<IMesh> IMesh::Create()
	{
#if defined RENDERER_VULKAN
		return CreateShared<Vulkan::Mesh>();
#endif
	}
}
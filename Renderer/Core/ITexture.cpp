#include "ITexture.h"

#include <Vulkan/Texture.h>

namespace Cosmos::Renderer
{
	Shared<ITexture2D> ITexture2D::Create(std::string path, bool gui)
	{
#if defined RENDERER_VULKAN
		return CreateShared<Vulkan::Texture2D>(path, gui);
#endif
	}

	Shared<ITextureCubemap> ITextureCubemap::Create(std::vector<std::string> paths)
	{
#if defined RENDERER_VULKAN
		return CreateShared<Vulkan::TextureCubemap>(paths);
#endif
	}
}
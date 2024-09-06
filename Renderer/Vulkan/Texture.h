#pragma once
#if defined RENDERER_VULKAN
#include "Wrapper/vulkan.h"

#include <Common/Util/Memory.h>
#include <string>
#include <vector>

namespace Cosmos::Renderer::Vulkan
{
	class Texture2D
	{
	public:

		// constructor
		Texture2D(std::string path);

		// destructor
		~Texture2D();

	public:

		// returns a reference to the image view
		virtual inline VkImageView GetView() { return mView; }

		// returns a reference to the image sampler
		virtual inline VkSampler GetSampler() { return mSampler; }

	private:

		// loads the texture based on constructor's path
		void LoadTexture();

		// creates mipmaps for the current bound texture
		void CreateMipmaps();

	private:

		std::string mPath = {};
		int32_t mWidth = 0;
		int32_t mHeight = 0;
		int32_t mMipLevels = 1;
		VkImage mImage = VK_NULL_HANDLE;
		VmaAllocation mMemory = VK_NULL_HANDLE;
		VkImageView mView = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;
	};

	class TextureCubemap
	{
	public:

		// constructor
		TextureCubemap(std::vector<std::string> paths);

		// destructor
		~TextureCubemap();

	public:

		// returns a reference to the image view
		inline VkImageView GetView() { return mView; }

		// returns a reference to the image sampler
		inline VkSampler GetSampler() { return mSampler; }

	private:

		// loads the texture based on constructor's path
		void LoadTextures();

	private:

		std::vector<std::string> mPaths = {};
		int32_t mWidth = 0;
		int32_t mHeight = 0;
		int32_t mMipLevels = 1;
		VkImage mImage = VK_NULL_HANDLE;
		VmaAllocation mMemory = VK_NULL_HANDLE;
		VkImageView mView = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;
	};
}
#endif
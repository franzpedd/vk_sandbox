#pragma once
#if defined RENDERER_VULKAN
#include "Wrapper/vulkan.h"

#include "Core/ITexture.h"

namespace Cosmos::Renderer::Vulkan
{
	class Texture2D : public ITexture2D
	{
	public:

		// constructor
		Texture2D(std::string path, bool gui = false);

		// constructor
		Texture2D(const BufferInfo& info, bool gui = false);

		// destructor
		virtual ~Texture2D();

	public:

		// returns a reference to the image view
		virtual void* GetView() override;

		// returns a reference to the image sampler
		virtual void* GetSampler() override;

		// returns an user-interface descriptor set of the image, used to display the texture on the ui
		virtual void* GetUIDescriptor() override;

	private:

		// loads the texture based on constructor's path
		void LoadTexture(bool gui);

		// loads the texture by a buffer data
		void LoadTextureFromBuffer(const BufferInfo& info, bool gui);

		// creates mipmaps for the current bound texture
		void CreateMipmaps();

	private:

		int32_t mWidth = 0;
		int32_t mHeight = 0;
		int32_t mMipLevels = 1;
		VkImage mImage = VK_NULL_HANDLE;
		VmaAllocation mMemory = VK_NULL_HANDLE;
		VkImageView mView = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;
		VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
	};

	class TextureCubemap : public ITextureCubemap
	{
	public:

		// constructor
		TextureCubemap(std::vector<std::string> paths);

		// destructor
		~TextureCubemap();

	public:

		// returns a reference to the image view
		virtual void* GetView() override;

		// returns a reference to the image sampler
		virtual void* GetSampler() override;

		// returns an user-interface descriptor set of the image, used to display the texture on the ui
		virtual void* GetUIDescriptor() override;

	private:

		// loads the texture based on constructor's path
		void LoadTextures();

	private:

		int32_t mWidth = 0;
		int32_t mHeight = 0;
		int32_t mMipLevels = 1;
		VkImage mImage = VK_NULL_HANDLE;
		VmaAllocation mMemory = VK_NULL_HANDLE;
		VkImageView mView = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;

		VkDescriptorSet mDescriptor = VK_NULL_HANDLE;
	};
}
#endif
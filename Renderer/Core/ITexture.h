#pragma once

#include <Common/Util/Memory.h>
#include <string>
#include <vector>

namespace Cosmos::Renderer
{
	class ITexture2D
	{
	public:

		// returns a smart-ptr to a new 2d texture
		static Shared<ITexture2D> Create(std::string path, bool gui = false);

		// constructor
		ITexture2D() = default;

		// destructor
		~ITexture2D() = default;

		// returns a reference to the texture's path
		inline std::string& GetPathRef() { return mPath; }

	public:

		// returns a reference to the image view
		virtual void* GetView() = 0;

		// returns a reference to the image sampler
		virtual void* GetSampler() = 0;

		// returns an user-interface descriptor set of the image, used to display the texture on the ui
		virtual void* GetUIDescriptor() = 0;

	protected:

		std::string mPath = {};
	};

	class ITextureCubemap
	{
	public:

		// returns a smart-ptr to a new cubemap texture
		static Shared<ITextureCubemap> Create(std::vector<std::string> paths);

		// constructor
		ITextureCubemap() = default;

		// destructor
		~ITextureCubemap() = default;

		// returns a reference to all texture paths
		inline std::vector<std::string>& GetPathsRef() { return mPaths; }

	public:

		// returns a reference to the image view
		virtual void* GetView() = 0;

		// returns a reference to the image sampler
		virtual void* GetSampler() = 0;

		// returns an user-interface descriptor set of the image, used to display the texture on the ui
		virtual void* GetUIDescriptor() = 0;

	protected:

		std::vector<std::string> mPaths = {};
	};
}
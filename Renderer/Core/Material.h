#pragma once

#include <Common/Util/Memory.h>
#include <string>

// forward declarations
namespace Cosmos::Renderer { class ITexture2D; }

namespace Cosmos::Renderer
{
	class Material
	{
	public:

		// constructor
		Material() = default;

		// destructor
		~Material() = default;

		// returns a reference to the material's name
		inline std::string& GetNameRef() { return mName; }

		// returns a reference to the material's albedo texture
		inline Shared<ITexture2D>& GetAlbedoTextureRef() { return mAlbedo; }

	private:

		std::string mName = {};
		Shared<ITexture2D> mAlbedo;
	};
}
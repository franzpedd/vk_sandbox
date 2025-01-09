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

		// returns the material's name
		inline std::string GetName() { return mName; }

		// sets the material's name
		inline void SetName(const std::string& name) { mName = name; }

		// returns a reference to the material's albedo texture
		inline Shared<ITexture2D>& GetAlbedoTextureRef() { return mAlbedo; }

	private:

		std::string mName = {};
		Shared<ITexture2D> mAlbedo;
	};
}
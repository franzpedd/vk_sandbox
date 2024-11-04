#pragma once

#include "Core/Material.h"
#include <Common/Math/BoundingBox.h>

namespace Cosmos::Renderer::GLTF
{
	class Primitive
	{
	public:

		// constructor
		Primitive(Material& material, uint32_t vertexCount, uint32_t indexCount, uint32_t firstIndex);

		// destructor
		~Primitive() = default;

		// returns a reference to the primitive material
		inline Material& GetMaterialRef() { return mMaterial; }

		// returns a reference to the primitive bounding box
		inline BoundingBox& GetBoundingBoxRef() { return mBoundingBox; }

		// returns either if the primitive has indices
		inline bool HasIndices() { return mIndexCount > 1; }

		// returns the first index of the primitive
		inline uint32_t GetFirstIndex() { return mFirstIndex; }

		// returns how many indices the primitive has
		inline uint32_t GetIndexCount() { return mIndexCount; }

		// returns how many vertices the primitive has
		inline uint32_t GetVertexCount() { return mVertexCount; }

	public:

		// calculates the bounding box of this primitive
		void SetBoundingBox(glm::vec3 min, glm::vec3 max);

	private:

		Material& mMaterial;
		BoundingBox mBoundingBox;
		uint32_t mFirstIndex = 0;
		uint32_t mIndexCount = 0;
		uint32_t mVertexCount = 0;
	};
}
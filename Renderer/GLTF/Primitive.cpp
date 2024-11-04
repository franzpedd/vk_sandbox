#include "Primitive.h"

namespace Cosmos::Renderer::GLTF
{
    Primitive::Primitive(Material &material,  uint32_t vertexCount, uint32_t indexCount, uint32_t firstIndex)
        : mMaterial(material),  mVertexCount(vertexCount), mIndexCount(indexCount), mFirstIndex(firstIndex)
    {
    }

    void Primitive::SetBoundingBox(glm::vec3 min, glm::vec3 max)
    {
		mBoundingBox.SetMin(min);
		mBoundingBox.SetMax(max);
		mBoundingBox.SetValid(true);
	}
}
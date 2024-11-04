#include "Skin.h"

#include "Node.h"

namespace Cosmos::Renderer::GLTF
{
    std::vector<Skin*> Skin::LoadSkins(const tinygltf::Model &model, std::vector<Node*>& nodesRef)
    {
        std::vector<Skin*> skins = {};

		for (const tinygltf::Skin& source : model.skins) {
			GLTF::Skin* newSkin = new GLTF::Skin();
			newSkin->GetNameRef() = source.name;

			// find skeleton root node
			if (source.skeleton > -1) {
				newSkin->SetSkeletonRoot(Node::GetNodeFromIndex(source.skeleton, nodesRef));
			}

			// find joint nodes
			for (int jointIndex : source.joints) {
				GLTF::Node* node = Node::GetNodeFromIndex(jointIndex, nodesRef);

				if (node) {
					newSkin->GetJointsRef().push_back(Node::GetNodeFromIndex(jointIndex, nodesRef));
				}
			}

			// get inverse bind matrices from buffer
			if (source.inverseBindMatrices > -1) {
				const tinygltf::Accessor& accessor = model.accessors[source.inverseBindMatrices];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
				newSkin->GetInverseBindMatrices().resize(accessor.count);
				memcpy(newSkin->GetInverseBindMatrices().data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
			}

			skins.push_back(newSkin);
		}

		return skins;
    }
}

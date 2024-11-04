#pragma once

#include "Wrapper/tinygltf.h"
#include <Common/Math/Math.h>
#include <string>
#include <vector>

// forward declarations
namespace Cosmos::Renderer::GLTF { class Node; }
namespace Cosmos::Renderer::Vulkan { class Device; }

namespace Cosmos::Renderer::GLTF
{
	class Skin
	{
	public:

		// constructor
		Skin() = default;

		// destructor
		~Skin() = default;

		// returns the root node that binds the skin together
		inline Node* GetSkeletonRoot() { return mSkeletonRoot; }

		// sets a new skeleton root node
		inline void SetSkeletonRoot(Node* node) { mSkeletonRoot = node; }

		// returns a reference to the skin name 
		inline std::string& GetNameRef() { return mName; }

		// returns the vector of joints the skin has
		inline std::vector<Node*>& GetJointsRef() { return mJoints; }

		// returns the vector of matrices in bone space
		inline std::vector<glm::mat4>& GetInverseBindMatrices() { return mInverseBindMatrices; }

	public:

		// loads and returns the model skins
		static std::vector<Skin*> LoadSkins(const tinygltf::Model& model, std::vector<Node*>& nodesRef);

	private:

		Node* mSkeletonRoot = nullptr;
		std::string mName;
		std::vector<Node*> mJoints;
		std::vector<glm::mat4> mInverseBindMatrices;
	};
}

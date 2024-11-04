#pragma once

#include "Core/Material.h"
#include "Core/Vertex.h"
#include "Wrapper/tinygltf.h"
#include <Common/Math/Math.h>
#include <Common/Math/BoundingBox.h>
#include <vector>

// forward declarations
namespace Cosmos::Renderer::GLTF { class Mesh; }
namespace Cosmos::Renderer::GLTF { class Skin; }

namespace Cosmos::Renderer::GLTF
{
	class Node
	{
	public:

		struct Boundaries
		{
			glm::mat4 matrix = glm::mat4(1.0f);
			glm::vec3 translation = glm::vec3(1.0f);
			glm::vec3 scale = glm::vec3(1.0f);
			glm::quat rotation = {};
			BoundingBox bvh;
			BoundingBox aabb;
		};

	public:

		// constructor
		Node(Node* parent, std::string name, uint32_t index, int32_t skinIndex);

		// destructor
		~Node() = default;

		// returns this node parent/father
		inline Node* GetParent() { return mParent; }

		// returns this node index/id
		inline uint32_t GetIndex() { return mIndex; }

		// returns a reference to the node name
		inline std::string& GetNameRef() { return mName; }

		// returns a reference to the node children
		inline std::vector<Node*>& GetChildrenRef() { return mChildren; }

		// returns the node mesh
		inline Mesh* GetMesh() { return mMesh; }

		// sets the node mesh
		inline void SetMesh(Mesh* mesh) { mMesh = mesh; }

		// returns the node skin
		inline Skin* GetSkin() { return mSkin; }

		// sets a new skin to the node
		inline void SetSkin(Skin* skin) { mSkin = skin; }

		// returns the skin index
		inline int32_t GetSkinIndex() { return mSkinIndex; }

		// returns the node boundaries/dimension
		inline Boundaries& GetBoundariesRef() { return mDimension;  }

	public:

		// updates the node
		void OnUpdate();

		// calculate the model matrix of this node only
		glm::mat4 GetLocalMatrix();

		// returns the model matrix of the chain of parent nodes, starting by this one
		glm::mat4 GetMatrix();

	public:

		struct MeshLoaderInfo
		{
			uint32_t* indexBuffer;
			Vertex* vertexBuffer;
			size_t indexPos = 0;
			size_t vertexPos = 0;
		};

		// returns the tinygltf node vertex and index count
		static void GetNodeVertexAndIndexCount(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount);

    	// loads the node and it's children into the vector of vertices
    	static void LoadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, MeshLoaderInfo& loader, std::vector<Node*>& nodes, std::vector<Node*>& linearNodes, Material& materialRef, std::vector<Vertex>& verticesRef, float globalScale = 1.0f);

		// returns the node index starting by the root of nodes
		static Node* GetNodeFromIndex(uint32_t index, std::vector<Node*>& nodesRef);

	private:

		// returns the node index, starting by a parent
		static Node* GetNode(Node* parent, uint32_t index);

	private:

		Node* mParent = nullptr;
		uint32_t mIndex = 0;
		std::string mName = {};
		std::vector<Node*> mChildren = {};
		Mesh* mMesh = nullptr;
		Skin* mSkin = nullptr;
		int32_t mSkinIndex = -1;

		Boundaries mDimension;
	};
}
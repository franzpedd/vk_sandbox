#include "Node.h"

#include "Mesh.h"
#include "Skin.h"
#include "Vulkan/Context.h"
#include <Common/Debug/Logger.h>

namespace Cosmos::Renderer::GLTF
{
	Node::Node(Node* parent, std::string name, uint32_t index, int32_t skinIndex)
		: mParent(parent), mName(name), mIndex(index), mSkinIndex(skinIndex)
	{
	}

	void Node::OnUpdate()
	{
		if (mMesh != nullptr) {
			glm::mat4 matrix = GetMatrix();

			if (mSkin != nullptr) {
				mMesh->GetUniformBlockRef().matrix = matrix;
				glm::mat4 inverseTransform = glm::inverse(matrix);
				size_t numJoints = std::min((uint32_t)mSkin->GetJointsRef().size(), COSMOS_MESH_MAX_JOINTS);

				for (size_t i = 0; i < numJoints; i++) {
					Node* jointNode = mSkin->GetJointsRef()[i];
					glm::mat4 jointMat = jointNode->GetMatrix() * mSkin->GetInverseBindMatrices()[i];
					jointMat = inverseTransform * jointMat;
					mMesh->GetUniformBlockRef().jointsMatrix[i] = jointMat;
				}

				mMesh->GetUniformBlockRef().jointsCount = static_cast<uint32_t>(numJoints);
				memcpy(mMesh->GetUniformBufferRef().mappedMemoryPtr, &mMesh->GetUniformBlockRef(), sizeof(Mesh::UniformBlock));
			}

			else {
				memcpy(mMesh->GetUniformBufferRef().mappedMemoryPtr, &matrix, sizeof(glm::mat4));
			}
		}

		for (auto& child : mChildren) {
			child->OnUpdate();
		}
	}

	glm::mat4 Node::GetLocalMatrix()
	{
		return glm::translate(glm::mat4(1.0f), mDimension.translation) * glm::mat4(mDimension.rotation) * glm::scale(glm::mat4(1.0f), mDimension.scale) * mDimension.matrix;
	}

	glm::mat4 Node::GetMatrix()
	{
		glm::mat4 m = GetLocalMatrix();
		Node* p = mParent;

		COSMOS_LOG(Logger::Warn, "This is only true if node's parent only has current node as a child");

		while (p) {
			m = p->GetLocalMatrix() * m;
			p = p->GetParent();
		}

		return m;
	}

    void Node::GetNodeVertexAndIndexCount(const tinygltf::Node& node, const tinygltf::Model& model, size_t& vertexCount, size_t& indexCount)
    {
        if (node.children.size() > 0) {
			for (size_t i = 0; i < node.children.size(); i++) {
				GetNodeVertexAndIndexCount(model.nodes[node.children[i]], model, vertexCount, indexCount);
			}
		}

		if (node.mesh > -1) {
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			for (size_t i = 0; i < mesh.primitives.size(); i++) {
				auto primitive = mesh.primitives[i];
				vertexCount += model.accessors[primitive.attributes.find("POSITION")->second].count;

				if (primitive.indices > -1) {
					indexCount += model.accessors[primitive.indices].count;
				}
			}
		}
    }

     void Node::LoadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, MeshLoaderInfo& loader, std::vector<Node*>& nodes, std::vector<Node*>& linearNodes, Material& materialRef, std::vector<Vertex>& verticesRef, float globalScale)
	{
		GLTF::Node* newNode = new Node(parent, node.name, nodeIndex, node.skin);

		// generate local node matrix
		glm::vec3 translation = glm::vec3(0.0f);
		glm::mat4 rotation = glm::mat4(1.0f);
		glm::vec3 scale = glm::vec3(1.0f);
		
		if (node.translation.size() == 3) {
			translation = glm::make_vec3(node.translation.data());
			newNode->GetBoundariesRef().translation = translation;
		}

		if (node.rotation.size() == 4) {
			glm::quat q = glm::make_quat(node.rotation.data());
			newNode->GetBoundariesRef().rotation = glm::mat4(q);
		}
		
		if (node.scale.size() == 3) {
			scale = glm::make_vec3(node.scale.data());
			newNode->GetBoundariesRef().scale = scale;
		}

		if (node.matrix.size() == 16) {
			newNode->GetBoundariesRef().matrix = glm::make_mat4x4(node.matrix.data());
		}

		// call children nodes
		for(size_t i = 0; i < node.children.size(); i++) {
			LoadNode(newNode, model.nodes[node.children[i]], node.children[i], model, loader, nodes, linearNodes, materialRef, verticesRef, globalScale);
		}

		// node contains mesh data
		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = model.meshes[node.mesh];

			Renderer::Vulkan::Context* renderer = (Renderer::Vulkan::Context*)(Renderer::IContext::GetRef());

			GLTF::Mesh* newMesh = new GLTF::Mesh(renderer->GetDevice(), newNode->GetMatrix());

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				const tinygltf::Primitive& primitive = mesh.primitives[j];
				uint32_t vertexStart = (uint32_t)(loader.vertexPos);
				uint32_t indexStart = (uint32_t)(loader.indexPos);
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;
				glm::vec3 posMin = {};
				glm::vec3 posMax = {};
				bool hasSkin = false;
				bool hasIndices = primitive.indices > -1;

				// vertices
				{
					const float* bufferPos = nullptr;
					const float* bufferNormals = nullptr;
					const float* bufferTexCoordSet0 = nullptr;
					const float* bufferTexCoordSet1 = nullptr;
					const float* bufferColorSet0 = nullptr;
					const void* bufferJoints = nullptr;
					const float* bufferWeights = nullptr;

					int posByteStride;
					int normByteStride;
					int uv0ByteStride;
					int color0ByteStride;
					int jointByteStride;
					int weightByteStride;

					int jointComponentType;

					// Position attribute is required
					assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

					const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
					bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
					posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
					posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
					vertexCount = (uint32_t)(posAccessor.count);
					posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
						const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
						bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
						normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// uv0
					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					// vertex colors
					if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
						const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
						bufferColorSet0 = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// skinning joints
					if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
						const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
						bufferJoints = &(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
						jointComponentType = jointAccessor.componentType;
						jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					// skinning weights
					if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
						const tinygltf::Accessor& weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
						const tinygltf::BufferView& weightView = model.bufferViews[weightAccessor.bufferView];
						bufferWeights = reinterpret_cast<const float*>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
						weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					hasSkin = (bufferJoints && bufferWeights);

					for (size_t v = 0; v < posAccessor.count; v++) {
						Vertex& vert = loader.vertexBuffer[loader.vertexPos];
						vert.position = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
						vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
						vert.uv = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
						vert.color = bufferColorSet0 ? glm::make_vec4(&bufferColorSet0[v * color0ByteStride]) : glm::vec4(1.0f);

						if (hasSkin) {
							switch (jointComponentType)
							{
								case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
								{
									const uint16_t* buf = static_cast<const uint16_t*>(bufferJoints);
									vert.joint = glm::uvec4(glm::make_vec4(&buf[v * jointByteStride]));
									break;
								}

								case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
								{
									const uint8_t* buf = static_cast<const uint8_t*>(bufferJoints);
									vert.joint = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
									break;
								}

								default:
								{
									COSMOS_LOG(Logger::Error, "Joint component type %d is not supported", jointComponentType);
									break;
								}
							}
						}

						else {
							vert.joint = glm::vec4(0.0f);
						}

						vert.weight = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);

						// fix for all zero weights
						if (glm::length(vert.weight) == 0.0f) {
							vert.weight = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
						}

						loader.vertexPos++;
						verticesRef[v] = vert;
					}
				}

				// indices
				if (hasIndices) {
					const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					indexCount = (uint32_t)(accessor.count);
					const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

					switch (accessor.componentType)
					{
						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
						{
							const uint32_t* buf = (const uint32_t*)(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								loader.indexBuffer[loader.indexPos] = buf[index] + vertexStart;
								loader.indexPos++;
							}

							break;
						}

						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
						{
							const uint16_t* buf = (const uint16_t*)(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								loader.indexBuffer[loader.indexPos] = buf[index] + vertexStart;
								loader.indexPos++;
							}

							break;
						}

						case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
						{
							const uint8_t* buf = (const uint8_t*)(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								loader.indexBuffer[loader.indexPos] = buf[index] + vertexStart;
								loader.indexPos++;
							}

							break;
						}

						default:
						{
							COSMOS_LOG(Logger::Error, "Inex component type %d is not supported");
							return;
						}
					}
				}

				GLTF::Primitive* newPrimitive = new GLTF::Primitive(materialRef, vertexCount, indexCount, indexStart);
				newPrimitive->SetBoundingBox(posMin, posMax);
				newMesh->GetPrimitivesRef().push_back(newPrimitive);
			}

			// set mesh bounding box from bounding boxes of primitives
			for (auto p : newMesh->GetPrimitivesRef()) {
				if (p->GetBoundingBoxRef().IsValid() && !newMesh->GetBoundingBoxRef().IsValid()) {
					newMesh->GetBoundingBoxRef() = p->GetBoundingBoxRef();
					newMesh->GetBoundingBoxRef().SetValid(true);;
				}

				newMesh->GetBoundingBoxRef().SetMin(glm::min(newMesh->GetBoundingBoxRef().GetMin(), p->GetBoundingBoxRef().GetMin()));
				newMesh->GetBoundingBoxRef().SetMax(glm::max(newMesh->GetBoundingBoxRef().GetMax(), p->GetBoundingBoxRef().GetMax()));
			}

			newNode->SetMesh(newMesh);
		}

		if (parent) {
			parent->GetChildrenRef().push_back(newNode);
		}

		else {
			nodes.push_back(newNode);
		}

		linearNodes.push_back(newNode);
	}

    Node *Node::GetNodeFromIndex(uint32_t index, std::vector<Node*>& nodesRef)
    {
        Node* found = nullptr;
		for (auto& node : nodesRef) {
			found = GetNode(node, index);

			if (found) {
				break;
			}
		}

		return found;
    }

    Node *Node::GetNode(Node* parent, uint32_t index)
    {
    	Node* found = nullptr;

		if (parent->GetIndex() == index) {
			return parent;
		}

		for (auto& child : parent->GetChildrenRef())
		{
			found = GetNode(child, index);

			if (found) {
				break;
			}
		}
		return found;
    }
}
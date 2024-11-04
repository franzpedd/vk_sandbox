#include "Animation.h"

#include "Node.h"
#include <Common/Debug/Logger.h>

namespace Cosmos::Renderer::GLTF
{
	glm::vec4 Animation::Sampler::CubicSplineInterpolation(size_t index, float time, uint32_t stride)
	{
		float delta = inputs[index + 1] - inputs[index];
		float t = (time - inputs[index]) / delta;
		const size_t current = index * stride * 3;
		const size_t next = (index + 1) * stride * 3;
		const size_t A = 0;
		const size_t V = stride * 1;
		const size_t B = stride * 2;

		float t2 = powf(t, 2);
		float t3 = powf(t, 3);
		glm::vec4 pt{ 0.0f };
		for (uint32_t i = 0; i < stride; i++) {
			float p0 = outputs[current + i + V];			// starting point at t = 0
			float m0 = delta * outputs[current + i + A];	// scaled starting tangent at t = 0
			float p1 = outputs[next + i + V];				// ending point at t = 1
			float m1 = delta * outputs[next + i + B];		// scaled ending tangent at t = 1
			pt[i] = ((2.f * t3 - 3.f * t2 + 1.f) * p0) + ((t3 - 2.f * t2 + t) * m0) + ((-2.f * t3 + 3.f * t2) * p1) + ((t3 - t2) * m0);
		}

		return pt;
	}

	void Animation::Sampler::Translate(size_t index, float time, Node* node)
	{
		switch (interpolation)
		{
			case Sampler::InterpolationType::LINEAR:
			{
				float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
				node->GetBoundariesRef().translation = glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
				break;
			}

			case Sampler::InterpolationType::STEP:
			{
				node->GetBoundariesRef().translation = outputsVec4[index];
				break;
			}

			case Sampler::InterpolationType::CUBICSPLINE:
			{
				node->GetBoundariesRef().translation = CubicSplineInterpolation(index, time, 3);
				break;
			}
		}
	}

    void Animation::Sampler::Scale(size_t index, float time, Node *node)
    {
		switch (interpolation)
		{
			case Sampler::InterpolationType::LINEAR:
			{
				float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
				node->GetBoundariesRef().scale = glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
				break;
			}
		
			case Sampler::InterpolationType::STEP:
			{
				node->GetBoundariesRef().scale = outputsVec4[index];
				break;
			}
		
			case Sampler::InterpolationType::CUBICSPLINE:
			{
				node->GetBoundariesRef().scale = CubicSplineInterpolation(index, time, 3);
				break;
			}
		}
    }

    void Animation::Sampler::Rotate(size_t index, float time, Node *node)
    {
		switch (interpolation)
		{
			case Sampler::InterpolationType::LINEAR:
			{
				float u = std::max(0.0f, time - inputs[index]) / (inputs[index + 1] - inputs[index]);
				glm::quat q1;
				q1.x = outputsVec4[index].x;
				q1.y = outputsVec4[index].y;
				q1.z = outputsVec4[index].z;
				q1.w = outputsVec4[index].w;
				glm::quat q2;
				q2.x = outputsVec4[index + 1].x;
				q2.y = outputsVec4[index + 1].y;
				q2.z = outputsVec4[index + 1].z;
				q2.w = outputsVec4[index + 1].w;
				node->GetBoundariesRef().rotation = glm::normalize(glm::slerp(q1, q2, u));
				break;
			}

			case Sampler::InterpolationType::STEP:
			{
				glm::quat q1;
				q1.x = outputsVec4[index].x;
				q1.y = outputsVec4[index].y;
				q1.z = outputsVec4[index].z;
				q1.w = outputsVec4[index].w;
				node->GetBoundariesRef().rotation = q1;
				break;
			}
			
			case Sampler::InterpolationType::CUBICSPLINE:
			{
				glm::vec4 rot = CubicSplineInterpolation(index, time, 4);
				glm::quat q;
				q.x = rot.x;
				q.y = rot.y;
				q.z = rot.z;
				q.w = rot.w;
				node->GetBoundariesRef().rotation = glm::normalize(q);
				break;
			}
		}
    }

	std::vector<Animation> Animation::LoadAnimations(const tinygltf::Model& model, std::vector<Node*>& nodes)
    {
		std::vector<Animation> animations = {};

		for (tinygltf::Animation anim : model.animations) {
			GLTF::Animation animation = {};

			if (anim.name.empty()) {
				animation.GetNameRef() = std::to_string(animations.size());
			}

			else {
				animation.GetNameRef() = anim.name;
			}

			// samplers
			for (auto& samp : anim.samplers) {
				GLTF::Animation::Sampler sampler = {};

				if (samp.interpolation == "LINEAR") {
					sampler.interpolation = GLTF::Animation::Sampler::InterpolationType::LINEAR;
				}

				if (samp.interpolation == "STEP") {
					sampler.interpolation = GLTF::Animation::Sampler::InterpolationType::STEP;
				}

				if (samp.interpolation == "CUBICSPLINE") {
					sampler.interpolation = GLTF::Animation::Sampler::InterpolationType::CUBICSPLINE;
				}

				// read sampler input time values
				{
					const tinygltf::Accessor& accessor = model.accessors[samp.input];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
					const float* buf = static_cast<const float*>(dataPtr);

					for (size_t index = 0; index < accessor.count; index++) {
						sampler.inputs.push_back(buf[index]);
					}

					for (auto input : sampler.inputs) {
						if (input < animation.GetStartTime()) {
							animation.SetStartTime(input);
						}

						if (input > animation.GetEndTime()) {
							animation.SetEndTime(input);
						}
					}
				}

				// read sampler output T/R/S values 
				{
					const tinygltf::Accessor& accessor = model.accessors[samp.output];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

					switch (accessor.type)
					{
						case TINYGLTF_TYPE_VEC3:
						{
							const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
								sampler.outputs.push_back(buf[index][0]);
								sampler.outputs.push_back(buf[index][1]);
								sampler.outputs.push_back(buf[index][2]);
							}

							break;
						}

						case TINYGLTF_TYPE_VEC4:
						{
							const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
							for (size_t index = 0; index < accessor.count; index++) {
								sampler.outputsVec4.push_back(buf[index]);
								sampler.outputs.push_back(buf[index][0]);
								sampler.outputs.push_back(buf[index][1]);
								sampler.outputs.push_back(buf[index][2]);
								sampler.outputs.push_back(buf[index][3]);
							}

							break;
						}

						default:
						{
							COSMOS_LOG(Logger::Error, "Unknown type of animation sampler output");
							break;
						}
					}
				}

				animation.GetSamplersRef().push_back(sampler);
			}

			// channels
			for (auto& source : anim.channels)
			{
				GLTF::Animation::Channel channel{};

				if (source.target_path == "rotation") {
					channel.path = GLTF::Animation::Channel::PathType::ROTATION;
				}

				if (source.target_path == "translation") {
					channel.path = GLTF::Animation::Channel::PathType::TRANSLATION;
				}

				if (source.target_path == "scale") {
					channel.path = GLTF::Animation::Channel::PathType::SCALE;
				}

				if (source.target_path == "weights") {
					COSMOS_LOG(Logger::Error, "Weights is not yet supported, skipping channel");
					continue;
				}

				channel.samplerIndex = source.sampler;
				channel.node = Node::GetNodeFromIndex(source.target_node, nodes);

				if (!channel.node) {
					continue;
				}

				animation.GetChannelsRef().push_back(channel);
			}

			animations.push_back(animation);
		}

        return animations;
    }
}
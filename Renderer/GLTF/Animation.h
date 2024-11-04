#pragma once

#include "Wrapper/tinygltf.h"
#include <Common/Math/Math.h>
#include <vector>

// forward declarations
namespace Cosmos::Renderer::GLTF { class Node; }

namespace Cosmos::Renderer::GLTF
{
	class Animation
	{
	public:

		struct Channel
		{
			enum PathType { TRANSLATION, ROTATION, SCALE };
			PathType path;
			Node* node;
			uint32_t samplerIndex;
		};

		struct Sampler
		{
			enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
			InterpolationType interpolation;
			std::vector<float> inputs;
			std::vector<glm::vec4> outputsVec4;
			std::vector<float> outputs;

			// calculates the cubic-spline interpolation
			glm::vec4 CubicSplineInterpolation(size_t index, float time, uint32_t stride);

			// translates the node
			void Translate(size_t index, float time, Node* node);

			// scales the node
			void Scale(size_t index, float time, Node* node);

			// rotates the node
			void Rotate(size_t index, float time, Node* node);
		};

	public:

		// constructor
		Animation() = default;

		// destructor
		~Animation() = default;

		// returns a reference to the animation name
		inline std::string& GetNameRef() { return mName; }

		// returns a reference to the animation channels
		inline std::vector<Channel>& GetChannelsRef() { return mChannels; }

		// returns a reference to the animation samplers
		inline std::vector<Sampler>& GetSamplersRef() { return mSamplers; }

		// returns the starting time
		inline float GetStartTime() { return mStart; }

		// sets a new starting time
		inline void SetStartTime(float time) { mStart = time; }

		// returns the ending time
		inline float GetEndTime() { return mEnd; }

		// sets a new ending time
		inline void SetEndTime(float time) { mEnd = time; }

	public:

		// loads and returns the model animations
    	static std::vector<Animation> LoadAnimations(const tinygltf::Model& model, std::vector<Node*>& nodesRef);

	private:

		std::string mName;
		std::vector<Channel> mChannels;
		std::vector<Sampler>  mSamplers;
		float mStart = std::numeric_limits<float>::max();
		float mEnd = std::numeric_limits<float>::min();
	};
}
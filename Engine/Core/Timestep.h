#pragma once

#include <chrono>

namespace Cosmos::Engine
{
	class Timestep
	{
	public:

		// constructor
		Timestep() = default;

		// destructor
		~Timestep() = default;

		// returns the average fps count
		inline uint32_t GetFramesPerSecond() const { return mLastFPS; }

		// returns the timestep
		inline float GetTimestep() const { return mTimestep; }

	public:

		// starts the frames per second count
		void StartFrame();

		// ends the frames per second count
		void EndFrame();

	private:

		std::chrono::high_resolution_clock::time_point mStart;
		std::chrono::high_resolution_clock::time_point mEnd;
		double mTimeDiff = 0.0f;
		float mFpsTimer = 0.0f;
		uint32_t mFrames = 0; // average fps
		float mTimestep = 1.0f; // timestep/delta time (used to update logic)
		uint32_t mLastFPS = 0;
	};
}
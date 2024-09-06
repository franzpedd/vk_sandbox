#pragma once

#include <chrono>

// forward declarations
namespace Cosmos::Engine { class Application; }

namespace Cosmos::Engine
{
	class Timestep
	{
	public:

		// constructor
		Timestep(Application* application);

		// destructor
		~Timestep() = default;

		// ends the timestep tick
		void OnTick();

		// returns the timestep
		inline double GetTimestep() { return mTimestep / 1000.0f; }

		// returns the average of frames per second
		inline float GetFramesPerSecond() { return (1000.0f / mTimestep); }

	private:

		Application* mApplication = nullptr;
		std::chrono::steady_clock::time_point mCurrentTime;
		double mDeltaTime = 0.03;
		double mAccumulator = 0.0;
		float mTimestep = 0.0f;
	};
}
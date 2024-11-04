#include "Timestep.h"

#include "Debug/Profiler.h"

namespace Cosmos::Engine
{
	void Timestep::StartFrame()
	{
		PROFILER_FUNCTION();
		mStart = std::chrono::high_resolution_clock::now();
	}

	void Timestep::EndFrame()
	{
		PROFILER_FUNCTION();

		mEnd = std::chrono::high_resolution_clock::now();	// ends timer
		mFrames++;											// add frame to the count
	
		// calculates time taken by the renderer updating
		mTimeDiff = std::chrono::duration<double, std::milli>(mEnd - mStart).count();
	
		mTimestep = (float)mTimeDiff / 1000.0f; // timestep
	
		// calculates time taken by last timestamp and renderer finished
		mFpsTimer += (float)mTimeDiff;
	
		if (mFpsTimer > 1000.0f) // greater than next frame, reset frame counting
		{
			mLastFPS = (uint32_t)((float)mFrames * (1000.0f / mFpsTimer));
			mFrames = 0;
			mFpsTimer = 0.0f;
		}
	}
}
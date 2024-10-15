#include "Timestep.h"

#include "Application.h"

namespace Cosmos::Engine
{
	Timestep::Timestep(Application* application)
		: mApplication(application)
	{
		mCurrentTime = std::chrono::steady_clock::now();
	}

	void Timestep::OnTick()
	{
		std::chrono::steady_clock::time_point newTime = std::chrono::steady_clock::now();
		mTimestep = std::chrono::duration<float, std::milli>(newTime - mCurrentTime).count();

		mCurrentTime = newTime;
		mAccumulator += mTimestep;

		while (mAccumulator >= mDeltaTime) {
			mApplication->OnSyncUpdate();
			mAccumulator -= mDeltaTime;
		}

		mApplication->OnAsyncUpdate();
	}
}
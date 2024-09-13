#include "Clock.h"

namespace Cosmos
{
	void Clock::Start()
	{
		mStart = std::chrono::steady_clock::now();
	}

	double Clock::Stop()
	{
		mEnd = std::chrono::steady_clock::now();
		auto diff = std::chrono::duration<double, std::milli>(mEnd - mStart).count();

		return diff;
	}
}

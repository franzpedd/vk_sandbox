#pragma once

#include <chrono>

namespace Cosmos
{
	class Timer
	{
	public:

		// constructor
		Timer() = default;

		// destructor
		~Timer() = default;

	public:

		// starts the clock
		void Start() {
			mStart = std::chrono::steady_clock::now();
		}

		// stop and returns the time taken since it's start
		double Stop() {
			mEnd = std::chrono::steady_clock::now();
			auto diff = std::chrono::duration<double, std::milli>(mEnd - mStart).count();

			return diff;
		}

	private:

		std::chrono::steady_clock::time_point mStart;
		std::chrono::steady_clock::time_point mEnd;
	};
}
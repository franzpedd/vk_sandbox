#pragma once

#include <chrono>

namespace Cosmos
{
	class Clock
	{
	public:

		// constructor
		Clock() = default;

		// destructor
		~Clock() = default;

	public:

		// starts the clock
		void Start();

		// stop and returns the time taken since it's start
		double Stop();

	private:
		
		std::chrono::steady_clock::time_point mStart;
		std::chrono::steady_clock::time_point mEnd;
	};
}
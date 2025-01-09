#pragma once

#include <cstdint>

namespace Cosmos::Platform
{
	// returns the ticks passed since window appeared
	uint64_t GameTicks();

	// returns the time in milli-seconds
	uint64_t GetGameTimeInSeconds();
}
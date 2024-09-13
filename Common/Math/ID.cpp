#include "ID.h"

#include <random>

namespace Cosmos
{
	static std::random_device sDevice;
	static std::mt19937_64 sEngine(sDevice());
	static std::uniform_int_distribution<uint64_t> sDistribution;

	ID::ID()
		: mID(sDistribution(sEngine))
	{
	}

	ID::ID(uint64_t id)
		: mID(id)
	{
	}

	ID::ID(std::string id)
	{
		mID = std::stoull(id);
	}
}
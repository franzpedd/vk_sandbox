#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>

namespace Cosmos
{
	class ID
	{
	public:

		struct Hash
		{
			size_t operator()(const ID& other) const
			{
				size_t hash = std::hash<uint64_t>()(other.mID);
				return hash;
			}
		};

	public:

		// constructor
		ID();

		// constructor with value
		ID(uint64_t id);

		// constructor with str value
		ID(std::string id);

		// destructor
		~ID() = default;

		// returns the uuid
		inline uint64_t GetValue() const { return mID; }

		// returns the id
		operator uint64_t() const { return mID; }

		// used for using unordered map with hashing
		bool operator==(const ID& id) const
		{
			if (this->mID == id) return true;
			else return false;
		}

		// used to stringfy uuid and use it as acessor in Scene entitymap
		operator std::string() const
		{
			return std::to_string(mID);
		}

	private:

		uint64_t mID;
	};
}
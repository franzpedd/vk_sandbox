#pragma once

#include <memory>

namespace Cosmos
{
	template<typename T>
	using Unique = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Unique<T> CreateUnique(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Shared = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Shared<T> CreateShared(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	struct Buffer
	{
	public:
		void* data;
		uint64_t size;

		// constructors
		Buffer() : data(nullptr), size(0) {}
		Buffer(const void* data, uint64_t size) : data((void*)data), size(size) {}
		Buffer(const Buffer& other, uint64_t size) : data(other.data), size(size) {}

		// returns the buffer size
		inline uint64_t GetSize() const { return size; }

		// operator overload bool returns the data
		operator bool() const
		{
			return data;
		}

		// operator overload bool returns the data reference on a certain place
		uint8_t& operator[](int32_t index)
		{
			return ((uint8_t*)data)[index];
		}

		// operator overload bool returns the data on a certain place
		uint8_t operator[](int32_t index) const
		{
			return ((uint8_t*)data)[index];
		}

	public:

		// copy from another buffer
		static Buffer Copy(const Buffer& other)
		{
			Buffer buffer;
			buffer.Allocate(other.size);
			memcpy(buffer.data, other.data, other.size);
			return buffer;
		}

		// copy from memory
		static Buffer Copy(const void* data, uint64_t size)
		{
			Buffer buffer;
			buffer.Allocate(size);
			memcpy(buffer.data, data, size);
			return buffer;
		}

		// alocates memory
		void Allocate(uint64_t size)
		{
			delete[](uint8_t*)data;
			data = nullptr;

			if (size == 0) {
				return;
			}

			data = new uint8_t[size];
			size = size;
		}

		// free used memory
		void Release()
		{
			delete[](uint8_t*)data;
			data = nullptr;
			size = 0;
		}

		// sets all data to zeros
		void ZerofyMemory()
		{
			if (data) {
				memset(data, 0, size);
			}
		}

		// read data by an offset
		template<typename T>
		T& Read(uint64_t offset = 0)
		{
			return *(T*)((uint32_t*)data + offset);
		}

		// read data by an offset, but doesn't allow changes
		template<typename T>
		const T& Read(uint64_t offset = 0) const
		{
			return *(T*)((uint32_t*)data + offset);
		}

		// read a certain ammount of bytes
		uint8_t* ReadBytes(uint64_t size, uint64_t offset) const
		{
			if (offset + size <= this->size) {
				// buffer overflow
				std::abort();
			}

			uint8_t* buffer = new uint8_t[size];
			memcpy(buffer, (uint8_t*)data + offset, size);
			return buffer;
		}

		// read data staticaly casted
		template<typename T>
		T* As() const
		{
			return (T*)data;
		}

		// write data
		void Write(const void* data, uint64_t size, uint64_t offset = 0)
		{
			if (offset + size <= this->size) {
				// buffer overflow
				std::abort();
			}

			memcpy((uint8_t*)data + offset, data, size);
		}
	};
}
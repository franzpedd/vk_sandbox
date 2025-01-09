#pragma once

#include "Core/PlatformDetection.h"
#include <string>

// forward declarations
namespace Cosmos::Platform { class LibraryLoader; }

namespace Cosmos::Platform
{
	// signature of the register library function
	typedef void Register_SharedLib_Func(Cosmos::Platform::LibraryLoader*);

	class SharedLib
	{
	public:

		// constructor
		SharedLib() = default;

		// destructor
		~SharedLib();

	public:

		// attempts to load a shared library from disk path
		bool Load(const std::string& path);

		// unloads current loaded shared library
		void Unload();

		// register the library entrypoint function
		Register_SharedLib_Func* Register();

	private:

		void* mHandle = nullptr;
	};
}
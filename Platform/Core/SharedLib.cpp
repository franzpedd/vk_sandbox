#include "SharedLib.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#elif defined PLATFORM_LINUX
#include <dlfcn.h>
#else
#error "Unsuported platform"
#endif

namespace Cosmos::Platform
{
	SharedLib::~SharedLib()
	{
		Unload();
	}

	bool SharedLib::Load(const std::string& path)
	{
#if defined PLATFORM_WINDOWS
		mHandle = (HMODULE)LoadLibraryA(path.c_str());
#elif defined PLATFORM_LINUX
		mHandle = dlopen(path.c_str(), RTLD_NOW);
#else
		#error "Unsuported Platform"
#endif
		return (mHandle != nullptr);
	}

	void SharedLib::Unload()
	{
#if defined PLATFORM_WINDOWS
		if (mHandle) { FreeLibrary((HMODULE)mHandle); }
#elif defined PLATFORM_LINUX
		if (mHandle) { dlclose(mHandle); }
#else
		#error "Unsuported Platform"
#endif
	}

	Register_SharedLib_Func* SharedLib::Register()
	{
#ifdef WIN32
		return reinterpret_cast<Register_SharedLib_Func*>(GetProcAddress((HMODULE)mHandle, "OnLoadLibrary"));
#elif defined PLATFORM_LINUX
		return reinterpret_cast<Register_SharedLib_Func*>(dlsym(mHandle, "OnLoadLibrary"));
#else
		#error "Unsuported Platform"
#endif
	}
}
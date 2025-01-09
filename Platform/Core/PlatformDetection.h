// windows platform
#if defined(_WIN64)
	#define PLATFORM_WINDOWS

// linux platform
#elif defined(__linux__) || defined(__gnu_linux__)
	#define PLATFORM_LINUX
	#if defined(__ANDROID__)
		#define PLATFORM_ANDROID
	#endif

// apple platform
#elif __APPLE__
	#define PLATFORM_APPLE
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR
		#define PLATFORM_IOS
		#define PLATFORM_IOS_SIMULATOR
	#elif TARGET_OS_IPHONE
		#define PLATFORM_IOS
	#else
		#error "Unknown Apple platform"
	#endif
#elif

// unix platform
#elif defined(__unix__)
	#define PLATFORM_UNIX

// posix platform
#elif defined(_POSIX_VERSION)
	#define PLATFORM_POSIX

// unknown platform
#else
	#error "Unknown platform!"
#endif


// This is used by the plugin system
#ifdef PLATFORM_WINDOWS
	#define API_EXPORT __declspec( dllexport )
	#define API_IMPORT __declspec( dllimport )
#else
	#define API_EXPORT
	#define API_IMPORT 
#endif
#pragma once

#include "Util/Memory.h"

#if defined(_WIN32)
#pragma warning(push)
#pragma warning(disable : 26800 26498 6285)
#endif
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif

#include <sstream>
#include <vector>

namespace Cosmos
{
	class Logger
	{
	public:

		typedef enum Severity
		{
			Trace = 0,
			Info,
			Warn,
			Error,
			Assert,

			MAX_SEVERITY
		} Severity;

		struct ConsoleMessage
		{
			Severity severity;
			std::string message;
		};

	public:

		// constructor
		Logger();

		// destructor
		~Logger();

		// returns the logger
		static Logger& GetInstance();

		// returns a reference to the backend logger (spdlogger)
		inline static Shared<spdlog::logger>& GetBackendLogger() { return s_Logger; }

		// logs to a ostringstream object
		template<class T>
		inline Logger& operator<<(const T& other)
		{
			mOutput << other << std::endl;
			return *this;
		}

		// returns the stored console messages
		inline std::vector<ConsoleMessage>& GetMessages() { return mConsoleMessages; }

	public:

		// outputs a message to os's terminal
		void ToTerminal(Severity severity, const char* file, int line, const char* msg, ...);

		// outputs a message to a file
		void ToFile(Severity severity, const char* path, const char* file, int line, const char* msg, ...);

		// translates severity level to readable text
		const char* SeverityToConstChar(Severity severity);

	private:

		std::ostringstream mOutput;
		std::vector<ConsoleMessage> mConsoleMessages;

		static Shared<spdlog::logger> s_Logger;
	};
}

// asserts are enabled on any build
#define COSMOS_ASSERT(x, ...)																							\
{																														\
	if(!(x))																											\
	{																													\
		Cosmos::Logger::GetBackendLogger()->critical(__VA_ARGS__);														\
		std::abort();																									\
	}																													\
}

// macros to facilitate using logging (only enabled on debug)
#define COSMOS_LOG(severity, ...)																						\
{																														\
	Cosmos::Logger::GetInstance().ToTerminal(severity, __FILE__, __LINE__, __VA_ARGS__);								\
}

#define COSMOS_LOG_FILE(severity, filepath, ...)												\
{																								\
	Cosmos::Logger::GetInstance().ToFile(severity, filepath, __FILE__, __LINE__, __VA_ARGS__);	\
	if(severity == Cosmos::Logger::Severity::Assert) std::abort();								\
}

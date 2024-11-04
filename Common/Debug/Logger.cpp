#include "Logger.h"

#include <cstdarg>

namespace Cosmos
{
	Shared<spdlog::logger> Logger::s_Logger;

	Logger::Logger()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(CreateShared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks[0]->set_pattern("%^[%T] %n: %v%$");

		s_Logger = CreateShared<spdlog::logger>("COSMOS", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_Logger);
		s_Logger->set_level(spdlog::level::trace);
		s_Logger->flush_on(spdlog::level::trace);
	}

	Logger::~Logger()
	{
		printf("%s", mOutput.str().c_str());
	}

	Logger& Logger::GetInstance()
	{
		static Logger instance;
		return instance;
	}

	void Logger::ToTerminal(Severity severity, const char* file, int line, const char* msg, ...)
	{
		constexpr unsigned int LOG_MAX_SIZE = 1024;
		char buffer[LOG_MAX_SIZE];

		va_list args;
		va_start(args, msg);
		vsnprintf(buffer, LOG_MAX_SIZE, msg, args);
		va_end(args);

		switch(severity) {																											
			case Cosmos::Logger::Severity::Trace: { s_Logger->trace(buffer);	break; }
			case Cosmos::Logger::Severity::Info: { s_Logger->info(buffer);	break; }
			case Cosmos::Logger::Severity::Warn: { s_Logger->warn(buffer);	break; }
			case Cosmos::Logger::Severity::Error: { s_Logger->error(buffer);	break; }
		}

		if (mConsoleMessages.size() >= LOG_MAX_SIZE) {
			mConsoleMessages.clear();
		}

		time_t now = time(NULL);
		char formatBuffer[40];
		struct tm tstruct = *localtime(&now);
		strftime(formatBuffer, sizeof(formatBuffer), "%X", &tstruct); // format: HH:MM:SS
		
		std::ostringstream oss;
		oss << "[" << formatBuffer << "]";
		oss << "[" << SeverityToConstChar(severity) << "]";
		oss << ": " << buffer << std::endl;
		mConsoleMessages.push_back({ severity, oss.str() });
	}

	void Logger::ToFile(Severity severity, const char* path, const char* file, int line, const char* msg, ...)
	{
		constexpr unsigned int LOG_MAX_SIZE = 1024;
		char buffer[LOG_MAX_SIZE];
		
		va_list args;
		va_start(args, msg);
		vsnprintf(buffer, LOG_MAX_SIZE, msg, args);
		va_end(args);

		time_t now = time(NULL);
		char formatBuffer[40];
		struct tm tstruct = *localtime(&now);
		strftime(formatBuffer, sizeof(formatBuffer), "%X", &tstruct); // format: HH:MM:SS

		std::stringstream oss;
		oss << "[" << formatBuffer << "]";
		oss << "[" << file << " - " << line << "]";
		oss << "[" << SeverityToConstChar(severity) << "]";
		oss << ": " << buffer;

		FILE* f = std::fopen(path, "a+");
		fprintf(f, "%s\n", oss.str().c_str());
		fclose(f);
	}

	const char* Logger::SeverityToConstChar(Severity severity)
	{
		switch (severity) {
			case Severity::Trace: return "Trace";
			case Severity::Info: return "Info";
			case Severity::Warn: return "Warning";
			case Severity::Error: return "Error";
			case Severity::Assert: return "Assertion";
		}

		return "Undefined Severity Level";
	}
}
#pragma once
#include <memory>
#include <string>
#include <string_view>
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Epoch
{
#define EPOCH_ENABLE_LOGGING _RUNTIME && !_DIST || !_RUNTIME

	class Log
	{
	public:
		enum class Level : uint8_t { Debug, Info, Warn, Error, Critical };
		
		static void Init();
		static void Shutdown();

		static std::shared_ptr<spdlog::logger>& GetLogger() { return staticLogger; }
		static std::shared_ptr<spdlog::logger>& GetAppConsoleLogger() { return staticEditorConsoleLogger; }

		template<typename... Args>
		static void PrintMessage(Log::Level aLevel, std::string_view aTag, Args&&... aArgs);

		template<typename... Args>
		static void PrintAppMessage(Log::Level aLevel, Args&&... aArgs);
		
		template<typename... Args>
		static void PrintAssertMessage(std::string_view aPrefix, Args&&... aArgs);

	private:
		inline static bool staticIsInitialized = false;
		inline static std::shared_ptr<spdlog::logger> staticLogger;
		inline static std::shared_ptr<spdlog::logger> staticEditorConsoleLogger;
	};
}

#if EPOCH_ENABLE_LOGGING

#define LOG_DEBUG(...)				::Epoch::Log::PrintMessage(::Epoch::Log::Level::Debug, "", __VA_ARGS__)
#define LOG_INFO(...)				::Epoch::Log::PrintMessage(::Epoch::Log::Level::Info, "", __VA_ARGS__)
#define LOG_WARNING(...)			::Epoch::Log::PrintMessage(::Epoch::Log::Level::Warn, "", __VA_ARGS__)
#define LOG_ERROR(...)				::Epoch::Log::PrintMessage(::Epoch::Log::Level::Error, "", __VA_ARGS__)
#define LOG_CRITICAL(...)			::Epoch::Log::PrintMessage(::Epoch::Log::Level::Critical, "", __VA_ARGS__)

#define LOG_DEBUG_TAG(tag, ...)		::Epoch::Log::PrintMessage(::Epoch::Log::Level::Debug, tag, __VA_ARGS__)
#define LOG_INFO_TAG(tag, ...)		::Epoch::Log::PrintMessage(::Epoch::Log::Level::Info, tag, __VA_ARGS__)
#define LOG_WARNING_TAG(tag, ...)	::Epoch::Log::PrintMessage(::Epoch::Log::Level::Warn, tag, __VA_ARGS__)
#define LOG_ERROR_TAG(tag, ...)		::Epoch::Log::PrintMessage(::Epoch::Log::Level::Error, tag, __VA_ARGS__)
#define LOG_CRITICAL_TAG(tag, ...)	::Epoch::Log::PrintMessage(::Epoch::Log::Level::Critical, tag, __VA_ARGS__)

#define CONSOLE_LOG_DEBUG(...)		::Epoch::Log::PrintAppMessage(::Epoch::Log::Level::Debug, __VA_ARGS__)
#define CONSOLE_LOG_INFO(...)		::Epoch::Log::PrintAppMessage(::Epoch::Log::Level::Info, __VA_ARGS__)
#define CONSOLE_LOG_WARN(...)		::Epoch::Log::PrintAppMessage(::Epoch::Log::Level::Warn, __VA_ARGS__)
#define CONSOLE_LOG_ERROR(...)		::Epoch::Log::PrintAppMessage(::Epoch::Log::Level::Error, __VA_ARGS__)

#else

#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_WARNING(...)
#define LOG_ERROR(...)
#define LOG_CRITICAL(...)

#define LOG_DEBUG_TAG(tag, ...)
#define LOG_INFO_TAG(tag, ...)
#define LOG_WARNING_TAG(tag, ...)
#define LOG_ERROR_TAG(tag, ...)
#define LOG_CRITICAL_TAG(tag, ...)

#define CONSOLE_LOG_DEBUG(...)
#define CONSOLE_LOG_INFO(...)
#define CONSOLE_LOG_WARN(...)
#define CONSOLE_LOG_ERROR(...)

#endif

namespace Epoch
{
	template<typename... Args>
	void Log::PrintMessage(Level aLevel, std::string_view aTag, Args&&... aArgs)
	{
		std::string logString = aTag.empty() ? "{}{}" : "[{}] {}";

		switch (aLevel)
		{
		case Level::Debug:
			GetLogger()->trace(logString, aTag, fmt::format(std::forward<Args>(aArgs)...));
			break;
		case Level::Info:
			GetLogger()->info(logString, aTag, fmt::format(std::forward<Args>(aArgs)...));
			break;
		case Level::Warn:
			GetLogger()->warn(logString, aTag, fmt::format(std::forward<Args>(aArgs)...));
			break;
		case Level::Error:
			GetLogger()->error(logString, aTag, fmt::format(std::forward<Args>(aArgs)...));
			break;
		case Level::Critical:
			GetLogger()->critical(logString, aTag, fmt::format(std::forward<Args>(aArgs)...));
			break;
		default:
			break;
		}
	}

	template<typename ...Args>
	inline void Log::PrintAppMessage(Level aLevel, Args && ...aArgs)
	{
		if (!GetAppConsoleLogger())
		{
			LOG_WARNING("App logger not initialized.");
			return;
		}

		switch (aLevel)
		{
		case Level::Debug:
			GetAppConsoleLogger()->trace(std::forward<Args>(aArgs)...);
			break;
		case Level::Info:
			GetAppConsoleLogger()->info(std::forward<Args>(aArgs)...);
			break;
		case Level::Warn:
			GetAppConsoleLogger()->warn(std::forward<Args>(aArgs)...);
			break;
		case Level::Error:
			GetAppConsoleLogger()->error(std::forward<Args>(aArgs)...);
			break;
		default:
			break;
		}
	}

	template<typename... Args>
	void Log::PrintAssertMessage(std::string_view aPrefix, Args&&... aArgs)
	{
		GetLogger()->error("{}: {}", aPrefix, fmt::format(std::forward<Args>(aArgs)...));

#ifndef _DIST
		std::string message = fmt::format(std::forward<Args>(aArgs)...);
		MessageBoxA(nullptr, message.c_str(), "Epoch Assert", MB_OK | MB_ICONERROR);
#endif
	}

	template<>
	inline void Log::PrintAssertMessage(std::string_view aPrefix)
	{
		GetLogger()->error("{}", aPrefix);

#ifndef _DIST
		MessageBoxA(nullptr, "No message :(", "Epoch Assert", MB_OK | MB_ICONERROR);
#endif
	}
}

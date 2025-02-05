#include "epch.h"
#include "Log.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Epoch/Editor/Console/EditorConsoleSink.h"
#include "Epoch/Utils/FileSystem.h"

namespace Epoch
{
#define HAS_CONSOLE !_DIST

	void Log::Init()
	{
#if EPOCH_ENABLE_LOGGING

		if (staticIsInitialized)
		{
			LOG_WARNING("Attempting to initialize logger twice!");
			return;
		}

		std::string logsDirectory = "logs";
		if (!FileSystem::Exists(logsDirectory))
		{
			FileSystem::CreateDirectory(logsDirectory);
		}

		std::vector<spdlog::sink_ptr> epochSinks =
		{
			std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/Epoch.log", true),
			#if HAS_CONSOLE
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
			#endif
		};

		epochSinks[0]->set_pattern("[%T] %l: %v%");
		#if HAS_CONSOLE
		epochSinks[1]->set_pattern("%^[%T] %v%$");
		#endif

		staticLogger = std::make_shared<spdlog::logger>("EPOCH", epochSinks.begin(), epochSinks.end());
		staticLogger->set_level(spdlog::level::trace);

		staticIsInitialized = true;

#endif
	}

	void Log::Shutdown()
	{
#if EPOCH_ENABLE_LOGGING

		staticLogger.reset();
		spdlog::drop("EPOCH");

#endif
	}

	void Log::InitAppConsole(bool aWithConsole)
	{
#if EPOCH_ENABLE_LOGGING

		std::vector<spdlog::sink_ptr> editorConsoleSinks;

		if (aWithConsole)
		{
			editorConsoleSinks =
			{
				std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/App.log", true),
				std::make_shared<EditorConsoleSink>(1)
			};

			editorConsoleSinks[0]->set_pattern("%l: %v");
			editorConsoleSinks[1]->set_pattern("%v");
		}
		else
		{
			editorConsoleSinks =
			{
				std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/App.log", true)
			};
			
			editorConsoleSinks[0]->set_pattern("%l: %v");
		}

		staticEditorConsoleLogger = std::make_shared<spdlog::logger>("CONSOLE", editorConsoleSinks.begin(), editorConsoleSinks.end());
		staticEditorConsoleLogger->set_level(spdlog::level::trace);

#endif
	}

	void Log::ShutdownAppConsole()
	{
#if EPOCH_ENABLE_LOGGING

		staticEditorConsoleLogger.reset();
		spdlog::drop("CONSOLE");

#endif
	}
}

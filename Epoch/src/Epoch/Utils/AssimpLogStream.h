#pragma once
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#include "Epoch/Debug/Log.h"

namespace Epoch
{
	struct AssimpLogStream : public Assimp::LogStream
	{
		static void Initialize()
		{
			if (Assimp::DefaultLogger::isNullLogger())
			{
				Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
				Assimp::DefaultLogger::get()->attachStream(new AssimpLogStream, Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Warn | Assimp::Logger::Err);
			}
		}

		void write(const char* message) override
		{
			std::string msg(message);
			if (!msg.empty() && msg[msg.length() - 1] == '\n')
			{
				msg.erase(msg.length() - 1);
			}
			if (strncmp(message, "Debug", 5) == 0)
			{
				LOG_DEBUG(msg);
			}
			else if (strncmp(message, "Info", 4) == 0)
			{
				LOG_INFO(msg);
			}
			else if (strncmp(message, "Warn", 4) == 0)
			{
				LOG_WARNING(msg);
			}
			else
			{
				LOG_ERROR(msg);
			}
		}
	};
}

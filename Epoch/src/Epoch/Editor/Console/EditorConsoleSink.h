#pragma once
#include <spdlog/sinks/base_sink.h>
#include <mutex>
#include "Epoch/Editor/Console/EditorConsolePanel.h"

namespace Epoch
{

	class EditorConsoleSink : public spdlog::sinks::base_sink<std::mutex>
	{
	public:
		explicit EditorConsoleSink(uint32_t aBufferCapacity) : myMessageBufferCapacity(aBufferCapacity), myMessageBuffer(aBufferCapacity) {}

		virtual ~EditorConsoleSink() = default;

		EditorConsoleSink(const EditorConsoleSink& aOther) = delete;
		EditorConsoleSink& operator=(const EditorConsoleSink& aOther) = delete;

	protected:
		void sink_it_(const spdlog::details::log_msg& aMsg) override
		{
			spdlog::memory_buf_t formatted;
			spdlog::sinks::base_sink<std::mutex>::formatter_->format(aMsg, formatted);
			std::string message = fmt::to_string(formatted);

			myMessageBuffer[myMessageCount++] = ConsoleMessage{Hash::GenerateFNVHash(message), message, GetMessageFlags(aMsg.level), std::chrono::system_clock::to_time_t(aMsg.time)};

			if (myMessageCount == myMessageBufferCapacity)
			{
				flush_();
			}
		}

		void flush_() override
		{
			for (const auto& message : myMessageBuffer)
			{
				EditorConsolePanel::PushMessage(message);
			}

			myMessageCount = 0;
		}

	private:
		static int16_t GetMessageFlags(spdlog::level::level_enum aLevel)
		{
			int16_t flags = 0;

			switch (aLevel)
			{
			case spdlog::level::trace:
			case spdlog::level::debug:
			{
				flags |= (int16_t)ConsoleMessageFlags::Debug;
				break;
			}
			case spdlog::level::info:
			{
				flags |= (int16_t)ConsoleMessageFlags::Info;
				break;
			}
			case spdlog::level::warn:
			{
				flags |= (int16_t)ConsoleMessageFlags::Warning;
				break;
			}
			case spdlog::level::err:
			case spdlog::level::critical:
			{
				flags |= (int16_t)ConsoleMessageFlags::Error;
				break;
			}
			}

			return flags;
		}

	private:
		uint32_t myMessageBufferCapacity;
		std::vector<ConsoleMessage> myMessageBuffer;
		uint32_t myMessageCount = 0;
	};

}

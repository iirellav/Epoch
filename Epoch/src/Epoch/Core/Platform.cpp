#include "epch.h"
#include "Platform.h"
#include <ctime>
#include <chrono>
#include "spdlog/fmt/chrono.h"

namespace Epoch::Platform
{
    uint64_t GetCurrentDateTimeU64()
    {
        std::string string = GetCurrentDateTimeString();
        return std::stoull(string);
    }

    std::string GetCurrentDateTimeString()
    {
		std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm* localTime = std::localtime(&currentTime);
		return fmt::format("{:%Y%m%d%H%M}", *localTime);
    }
}

#pragma once
#include <string>
#include <ctime>

namespace Epoch
{

	enum class ConsoleMessageFlags : int16_t
	{
		None	= 0,
		Debug	= 1u << 0,
		Info	= 1u << 1,
		Warning = 1u << 2,
		Error	= 1u << 3,

		All = Debug | Info | Warning | Error
	};

	struct ConsoleMessage
	{
		uint64_t hash;
		std::string message;
		int16_t flags;
		time_t time;
	};
	
}

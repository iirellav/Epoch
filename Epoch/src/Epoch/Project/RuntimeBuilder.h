#pragma once
#include <filesystem>

namespace Epoch
{
	class RuntimeBuilder
	{
	public:
		RuntimeBuilder() = default;

		bool Build();
	};
}

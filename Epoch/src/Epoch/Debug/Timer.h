#pragma once
#include <chrono>

namespace Epoch
{
	class Timer
	{
	public:
		Timer() { Reset(); }
		void Reset() { myStart = std::chrono::high_resolution_clock::now(); }
		float Elapsed() { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - myStart).count() * 0.001f * 0.001f; }
		float ElapsedMillis() { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - myStart).count() * 0.001f; }

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> myStart;
	};
}

#pragma once
#include <chrono>

namespace CU
{
	class Timer
	{
	public:
		Timer() = delete;
		~Timer() = delete;

		static void Init();
		static void Reset();
		static void Update();

		static float GetDeltaTime();
		static double GetTotalTime();

	private:
		inline static std::chrono::high_resolution_clock::time_point myStartTime;
		inline static std::chrono::high_resolution_clock::time_point myLastTime;
		inline static float myDeltaTime;
	};
}

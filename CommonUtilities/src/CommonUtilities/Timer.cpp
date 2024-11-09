#include "Timer.h"
#include "Math/CommonMath.hpp"

namespace CU
{
	void Timer::Init()
	{
		Reset();
	}

	void Timer::Reset()
	{
		myStartTime = std::chrono::high_resolution_clock::now();
		myLastTime = std::chrono::high_resolution_clock::now();
	}

	void Timer::Update()
	{
		myDeltaTime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - myLastTime).count();
		myLastTime = std::chrono::high_resolution_clock::now();
	}

	float Timer::GetDeltaTime()
	{
		return myDeltaTime;
	}

	double Timer::GetTotalTime()
	{
		return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - myStartTime).count();
	}
}

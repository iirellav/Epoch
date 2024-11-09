#pragma once
#include <filesystem>
#include <chrono>
#include <thread>
#include <mutex>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "Log.h"

namespace Epoch
{
	using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

	struct ProfileResult
	{
		std::string name;

		FloatingPointMicroseconds start;
		std::chrono::microseconds elapsedTime;
		std::thread::id threadID;
	};

	struct InstrumentationSession
	{
		std::string name;
		std::string outputPath;
	};

	class Instrumentor
	{
	public:
		static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}

		void OpenStream()
		{
			if (myOutputStream.is_open()) return;

			if (!myIsPaused)
			{
				LOG_INFO("Instrumentor opening stream '{}'.", myCurrentSession->name);
				//std::lock_guard lock(myMutex);

				if (!std::filesystem::exists(myCurrentSession->outputPath))
				{
					std::filesystem::create_directories(std::filesystem::path(myCurrentSession->outputPath).remove_filename());
				}

				myOutputStream.open(myCurrentSession->outputPath);
				
				if (myOutputStream.is_open())
				{
					WriteHeader();
				}
				else
				{
					LOG_ERROR("Instrumentor could not open results file '{}'.", myCurrentSession->outputPath);
				}
			}
		}

		void BeginSession(const std::string& aName, const std::string& aFilepath = "tracing_results.json", bool aPaused = false)
		{
			std::lock_guard lock(myMutex);
			if (myCurrentSession)
			{
				LOG_ERROR("Instrumentor::BeginSession('{}') when session '{}' already open.", aName, myCurrentSession->name);

				InternalEndSession();
			}

			myIsPaused = aPaused;

			myCurrentSession = new InstrumentationSession({ aName, aFilepath });
			OpenStream();

			//if (!std::filesystem::exists(aFilepath))
			//{
			//	std::filesystem::create_directories(std::filesystem::path(aFilepath).remove_filename());
			//}

			//myOutputStream.open(aFilepath);
			//
			//if (myOutputStream.is_open())
			//{
			//	myCurrentSession = new InstrumentationSession({ aName });
			//	WriteHeader();
			//}
			//else
			//{
			//	LOG_ERROR("Instrumentor could not open results file '{}'.", aFilepath);
			//}
		}

		void EndSession()
		{
			std::lock_guard lock(myMutex);
			InternalEndSession();
		}

#pragma warning( disable : 26115 )
		void WriteProfile(const ProfileResult& result)
		{
			std::stringstream json;

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.elapsedTime.count()) << ',';
			json << "\"name\":\"" << result.name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.threadID << ",";
			json << "\"ts\":" << result.start.count();
			json << "}";

			std::lock_guard lock(myMutex);
			if (myCurrentSession)
			{
				myOutputStream << json.str();
				myOutputStream.flush();
			}
		}
#pragma warning(default:4700) 

		bool IsPaused() { return myIsPaused; }
		void Pause() { myIsPaused = true; }
		void Unpause() { myIsPaused = false; std::lock_guard lock(myMutex); OpenStream(); }

	private:
		Instrumentor() : myCurrentSession(nullptr) {}

		~Instrumentor()
		{
			EndSession();
		}

		void WriteHeader()
		{
			myOutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
			myOutputStream.flush();
		}

		void WriteFooter()
		{
			myOutputStream << "]}";
			myOutputStream.flush();
		}

		void InternalEndSession()
		{
			if (myCurrentSession)
			{
				WriteFooter();
				myOutputStream.close();
				delete myCurrentSession;
				myCurrentSession = nullptr;
			}
		}

	private:
		std::mutex myMutex;
		InstrumentationSession* myCurrentSession;
		std::ofstream myOutputStream;

		bool myIsPaused = false;
	};

	class InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* aName) : myName(aName), myStopped(false)
		{
			if (Instrumentor::Get().IsPaused())
			{
				myStopped = true;
				return;
			}

			myStartTimepoint = std::chrono::steady_clock::now();
		}

		~InstrumentationTimer()
		{
			if (!myStopped)
			{
				Stop();
			}
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::steady_clock::now();
			auto highResStart = std::chrono::duration<double, std::micro>{ myStartTimepoint.time_since_epoch() };
			auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(myStartTimepoint).time_since_epoch();

			Instrumentor::Get().WriteProfile({ myName, highResStart, elapsedTime, std::this_thread::get_id() });

			myStopped = true;
		}

	private:
		const char* myName;
		std::chrono::time_point<std::chrono::steady_clock> myStartTimepoint;
		bool myStopped;
	};

	namespace InstrumentorUtils 
	{

		template <size_t N>
		struct ChangeResult
		{
			char Data[N];
		};

		template <size_t N, size_t K>
		constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
		{
			ChangeResult<N> result = {};

			size_t srcIndex = 0;
			size_t dstIndex = 0;
			while (srcIndex < N)
			{
				size_t matchIndex = 0;
				while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
				{
					matchIndex++;
				}
				if (matchIndex == K - 1)
				{
					srcIndex += matchIndex;
				}

				result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
				srcIndex++;
			}

			return result;
		}
	}

#define EPOCH_PROFILE 1
#if EPOCH_PROFILE
/// INTERNAL ///
#define EPOCH_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = InstrumentorUtils::CleanupOutputString(name, "__cdecl "); InstrumentationTimer timer##line(fixedName##line.Data)
#define EPOCH_PROFILE_SCOPE_LINE(name, line) EPOCH_PROFILE_SCOPE_LINE2(name, line)
////////////////

#define EPOCH_PROFILE_BEGIN_SESSION(name, filepath, paused) Epoch::Instrumentor::Get().BeginSession(name, filepath, paused)
#define EPOCH_PROFILE_END_SESSION() Epoch::Instrumentor::Get().EndSession()

#define EPOCH_PROFILE_SCOPE(name) EPOCH_PROFILE_SCOPE_LINE(name, __LINE__)
#define EPOCH_PROFILE_FUNC() EPOCH_PROFILE_SCOPE(__FUNCSIG__)

#define EPOCH_PAUSE_PROFILING() Epoch::Instrumentor::Get().Pause()
#define EPOCH_UNPAUSE_PROFILING() Epoch::Instrumentor::Get().Unpause()
#else
#define EPOCH_PROFILE_SCOPE_LINE2(name, line)
#define EPOCH_PROFILE_SCOPE_LINE(name, line)

#define EPOCH_PROFILE_BEGIN_SESSION(name, filepath, paused)
#define EPOCH_PROFILE_END_SESSION()

#define EPOCH_PROFILE_SCOPE(name)
#define EPOCH_PROFILE_FUNCTION()

#define EPOCH_PAUSE_PROFILING()
#define EPOCH_UNPAUSE_PROFILING()
#endif
}

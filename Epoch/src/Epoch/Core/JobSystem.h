#pragma once
#include <Windows.h>
#include <memory>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include "Epoch/Debug/Assert.h"
#include "Epoch/Debug/Profiler.h"

namespace Epoch
{
	class JobSystem
	{
	public:
		static inline unsigned WorkerCount = 0;

	public:
		void Init()
		{
			EPOCH_ASSERT(WorkerCount == 0, "Can't instantiate a/the job system once!");

			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			WorkerCount = sysinfo.dwNumberOfProcessors;
			WorkerCount = (unsigned)((float)WorkerCount * 0.8f);

			EPOCH_ASSERT(myJobs.empty());
			myRunning = true;

			for (unsigned i = 0; i < WorkerCount; ++i)
			{
				myWorkers.emplace_back([this]
					{
						while (true)
						{
							std::function<void()> job;

							{
								std::unique_lock<std::mutex> lock(myMutex);
								myCondition.wait(lock, [this] { return !myJobs.empty() || !myRunning; });

								if (!myRunning && myJobs.empty())
								{
									return;
								}

								job = std::move(myJobs.front());
								myJobs.pop();
								++myActiveTasks;
							}

							job();

							{
								std::lock_guard<std::mutex> lock(myMutex);
								--myActiveTasks;
							}
						}
					});
			}

			LOG_INFO("Job System initialized with {} threads", WorkerCount);
		}

		~JobSystem()
		{
			EPOCH_PROFILE_FUNC();

			{
				std::unique_lock<std::mutex> lock(myMutex);
				myRunning = false;
			}
			myCondition.notify_all();
			for (std::thread& worker : myWorkers)
			{
				worker.join();
			}
		}

		template<typename F, typename... Args>
		auto AddAJob(F&& aFunction, Args&&... aArgs) -> std::future<decltype(aFunction(aArgs...))>
		{
			using return_type = decltype(aFunction(aArgs...));
			auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(aFunction), std::forward<Args>(aArgs)...));
			std::future<return_type> result = task->get_future();

			{
				std::unique_lock<std::mutex> lock(myMutex);
				myJobs.emplace([task]() { (*task)(); });
			}
			myCondition.notify_one();

			return result;
		}

		bool AllTasksDone()
		{
			std::lock_guard<std::mutex> lock(myMutex);
			return myActiveTasks == (uint32_t)0 && myJobs.empty();
		}

		void WaitUntilDone()
		{
			EPOCH_PROFILE_FUNC();
			while (!AllTasksDone())
			{
				//std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

	private:
		std::vector<std::thread> myWorkers;
		std::queue<std::function<void()>> myJobs;
		std::mutex myMutex;
		std::condition_variable myCondition;
		bool myRunning = false;
		uint32_t myActiveTasks = 0;
	};
}

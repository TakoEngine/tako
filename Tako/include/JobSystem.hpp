#pragma once
#include "Utility.hpp"
#include <thread>
#include <atomic>
#include <vector>
#include <deque>
#include <chrono>

namespace tako
{
	class JobSystem;
	class Job
	{
	public:
		Job(std::function<void()>&& func)
		{
			m_func = std::move(func);
		}
	private:
		friend class JobSystem;
		std::function<void()> m_func;
		Job* m_continuation = nullptr;
		std::atomic<int> m_jobsLeft = 1;
	};

	class JobQueue
	{
	public:
		JobQueue() noexcept {}
		JobQueue(const JobQueue&) noexcept {};
		~JobQueue() {}
		void Push(Job* job)
		{
			while (m_lock.test_and_set(std::memory_order_acquire));
			m_queue.push_back(job);
			m_lock.clear(std::memory_order_release);
		}

		Job* Pop()
		{
			Job* job = nullptr;
			while (m_lock.test_and_set(std::memory_order_acquire));
			if (!m_queue.empty())
			{
				job = m_queue.front();
				m_queue.pop_front();
			}
			
			m_lock.clear(std::memory_order_release);
			return job;
		}
	private:
		std::deque<Job*> m_queue;
		std::atomic_flag m_lock = ATOMIC_FLAG_INIT;
	};

	class JobSystem
	{
	public:
		JobSystem()
		{
		}

		void Init()
		{
			m_threadIndex = 0;
			m_threadCount = std::thread::hardware_concurrency();
			LOG("Threads: {}", m_threadCount);
			m_localQueues.reserve(m_threadCount);
			m_globalQueues.reserve(m_threadCount);
			for (unsigned int i = 0; i < m_threadCount; i++)
			{
				m_localQueues.emplace_back();
				m_globalQueues.emplace_back();
			}
			
			for (unsigned int i = 1; i < m_threadCount; i++)
			{
				std::thread thread(&JobSystem::WorkerThread, this, i);
				thread.detach();
				m_workers.push_back(std::move(thread));
			}
		}

		void JoinAsWorker()
		{
			WorkerThread(0);
		}

		void Stop()
		{
			m_stop = true;
		}

		Job* Schedule(std::function<void()>&& job)
		{
			auto allocatedJob = new Job(std::move(job));
			return Schedule(allocatedJob);
		}

		Job* Schedule(Job* job)
		{
			m_globalQueues[m_threadIndex].Push(job);
			return job;
		}

		Job* ScheduleForThread(unsigned int thread, std::function<void()>&& job)
		{
			auto allocatedJob = new Job(std::move(job));
			return ScheduleForThread(thread, allocatedJob);
		}

		Job* ScheduleForThread(unsigned int thread, Job* job)
		{
			m_localQueues[m_threadIndex].Push(job);
			return job;
		}

		static void Continuation(std::function<void()>&& job)
		{
			m_runningJob->m_continuation = new Job(std::move(job));
		}

	private:
		std::vector<std::thread> m_workers;
		std::vector<JobQueue> m_localQueues;
		std::vector<JobQueue> m_globalQueues;
		std::atomic<bool> m_stop = false;
		static inline thread_local Job* m_runningJob = nullptr;
		static inline thread_local unsigned int m_threadIndex;
		unsigned int m_threadCount;

		void WorkerThread(unsigned int threadIndex)
		{
			m_threadIndex = threadIndex;
			while (!m_stop)
			{
				auto job = m_localQueues[m_threadIndex].Pop();
				for (unsigned int i = 0; job == nullptr && i < m_threadCount; i++)
				{
					job = m_globalQueues[(i + m_threadIndex) % m_threadCount].Pop();
				}
				if (job != nullptr)
				{
					m_runningJob = job;
					LOG("Start job({})", m_threadIndex);
					job->m_func();
					job->m_jobsLeft--;
					if (job->m_jobsLeft <= 0)
					{
						if (job->m_continuation)
						{
							ScheduleForThread(0, job->m_continuation); //TODO: save target thread
						}
						delete job;
					}
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				
			}
		}
	};
}

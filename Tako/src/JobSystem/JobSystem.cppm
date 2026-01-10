module;
#include "Utility.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <optional>
#include <coroutine>
#include <queue>
#include <mutex>
#include <condition_variable>
export module Tako.JobSystem;

import Tako.Allocators.FreeListAllocator;
import Tako.Allocators.PoolAllocator;


namespace tako
{
	export class JobSystem;

	template<typename R, bool IsVoid = std::is_same_v<void, R>>
	struct PromiseBase;

	template<typename R>
	struct PromiseBase<R, true>
	{
		void return_void() noexcept {}
	};

	template<typename R>
	struct PromiseBase<R, false>
	{
		void return_value(R&& value) noexcept
		{
			result = std::move(value);
		}

		std::optional<R> result;
	};

	export template<typename R>
	class Task;

	struct InitialTaskAwaiter
	{
		constexpr bool await_ready() const noexcept { return false; }

		template<typename Promise>
		constexpr void await_suspend(std::coroutine_handle<Promise> handle) const noexcept;

		constexpr void await_resume() const noexcept {}
	};

	template<typename R = void>
	struct Promise : public PromiseBase<R>
	{
		InitialTaskAwaiter initial_suspend() noexcept { return {}; }
		std::suspend_always final_suspend() noexcept
		{
			task->m_done = true;
			if (task->m_parent)
			{
				task->m_parent->OnChildFinished(task);
			}
			return {};
		}

		Task<R> get_return_object()
		{
			return Task<R>(std::coroutine_handle<Promise<R>>::from_promise(*this));
		}

		void unhandled_exception() {}

		Task<R>* task;
	};

	class Job
	{
		friend JobSystem;
		template<typename R>
		friend struct TaskAwaiter;
		template<typename R>
		friend struct Promise;
	public:
		virtual bool Done() = 0;
	protected:
		virtual void Run() = 0;
		virtual void OnChildAwait(Job* child) = 0;
		virtual void OnChildFinished(Job* child) = 0;

		Job* m_parent = nullptr;
	};

	template<typename R = void>
	struct TaskAwaiter
	{
		Task<R>* task;

		TaskAwaiter(Task<R>* task) : task(task) {}

		bool await_ready() const noexcept
		{
			//return false;

			//TODO: Investigate spuriously double free of task,
			// when the coroutine is allowed to continue when the task is done
			return task->Done();
		}
		void await_suspend(std::coroutine_handle<> handle) const noexcept
		{
			ASSERT(task->m_parent);
			task->m_parent->OnChildAwait(task);
		}

		auto await_resume() const noexcept
		{
			return task->GetResult();
		}
	};

	template<typename R = void>
	class Task : public Job
	{
	public:
		using promise_type = Promise<R>;
		friend promise_type;


		constexpr explicit Task(std::coroutine_handle<promise_type> handle) noexcept : m_handle(handle)
		{
			handle.promise().task = this;
		}

		Task(const Task&) = delete;
		Task& operator= (const Task&) = delete;

		constexpr ~Task() noexcept
		{
			//TODO: figure out root cause for double free, instead of preventing the damage
			bool destroyed = m_destroyed;
			if (!destroyed && !m_destroyed.compare_exchange_strong(destroyed, true))
			{
				m_handle.destroy();
				m_handle = {};
			}
		}

		bool Done() override
		{
			return m_done;
		}

		auto GetResult()
		{
			if constexpr (!std::is_void_v<R>)
			{
				return m_handle.promise().result.value();
			}
		}

		auto operator co_await()
		{
			return TaskAwaiter<R>(this);
		}
	protected:
		void Run() override
		{
			if (!m_done)
			{
				m_handle.resume();
			}
		}

		void OnChildAwait(Job* child) override
		{
			m_waitingFor = child;
			if (child->Done())
			{
				// In case the child completed before waitingFor was assigned,
				// check if reschedule is required
				CheckRescheduleAfterAwait(child);
			}
		}

		void OnChildFinished(Job* child) override
		{
			CheckRescheduleAfterAwait(child);
		}
	private:
		std::atomic<bool> m_done = false;
		std::atomic<bool> m_destroyed = false;
		std::atomic<Job*> m_waitingFor = nullptr;
		std::coroutine_handle<promise_type> m_handle;

		void CheckRescheduleAfterAwait(Job* child);
	};

	class JobSystem
	{
		template<typename R>
		friend class Task;
		friend struct InitialTaskAwaiter;
	public:
		JobSystem()
		{
		}

		void Init()
		{
			m_threadIndex = 0;
#ifdef EMSCRIPTEN
			m_threadCount = emscripten_run_script_int("navigator.hardwareConcurrency");
#else
			m_threadCount = std::thread::hardware_concurrency();
#endif
			LOG("Threads: {}", m_threadCount);

			int workerTarget = m_threadCount - 1;
			m_workers.resize(workerTarget);
			for (unsigned int i = 0; i < workerTarget; i++)
			{
				int threadIndex = i + 1;
				LOG("Creating Thread: {}", threadIndex);
				std::thread& thread = m_workers[i] = std::thread(&JobSystem::WorkerThread, this, threadIndex);
				thread.detach();
			}
		}

		void Stop()
		{
			m_stop = true;
			m_globalCV.notify_all();
		}

		template<typename R>
		R Start(Task<R>&& mainTask)
		{
			auto mainJob = JobSystem::m_runningJob = GetJob(); // Assume it's scheduled already
			ASSERT(mainJob == &mainTask);
			m_started = true;
			m_started.notify_all();
			mainJob->Run();
			m_runningJob = nullptr;
			while (!m_stop && !mainJob->Done())
			{
				{
					Job* job = nullptr;
					bool moreMainThreadJobs = false;
					{
						std::lock_guard<std::mutex> lock(m_mainThreadQueueMutex);
						if (!m_mainThreadQueue.empty())
						{
							job = m_mainThreadQueue.front();
							m_mainThreadQueue.pop_front();
						}
					}
					RunJob(job);
					if (job)
					{
						continue;
					}
				}

				Job* job  = nullptr;
				{
					std::lock_guard<std::mutex> lock(m_globalQueueMutex);
					if (!m_globalQueue.empty())
					{
						job = m_globalQueue.front();
						m_globalQueue.pop_front();
					}
				}
				RunJob(job);
			}

			return mainTask.GetResult();
		}

		template<typename Cb, typename... Args>
			requires std::invocable<Cb, Args...>
		static Task<std::invoke_result_t<Cb, Args...>> Taskify(Cb func, Args&&... args)
		{
			co_return func(std::forward<Args>(args)...);
		}

		static void ScheduleNextTaskOnMain()
		{
			m_scheduleNextTaskOnMain = true;
		}
	private:
		std::vector<std::thread> m_workers;
		std::atomic<bool> m_started = false;
		std::atomic<bool> m_stop = false;

		static inline thread_local unsigned int m_threadIndex;
		static inline unsigned int m_threadCount;
		static inline std::deque<Job*> m_globalQueue;
		static inline std::mutex m_globalQueueMutex;
		static inline std::condition_variable m_globalCV;
		static inline std::deque<Job*> m_mainThreadQueue;
		static inline std::mutex m_mainThreadQueueMutex;
		static inline thread_local Job* m_runningJob = nullptr;
		static inline thread_local bool m_scheduleNextTaskOnMain = false;

		static void ScheduleJob(Job* job)
		{
			job->m_parent = JobSystem::m_runningJob;
			ReScheduleJob(job);
		}

		static void ReScheduleJob(Job* job)
		{
			ASSERT(!job->Done());
			if (m_scheduleNextTaskOnMain)
			{
				m_scheduleNextTaskOnMain = false;
				PushMainThreadJob(job);
			}
			else
			{
				PushGlobalJob(job);
			}
		}

		static void PushGlobalJob(Job* job)
		{
			{
				std::lock_guard<std::mutex> lock(m_globalQueueMutex);
				m_globalQueue.push_back(job);
			}
			m_globalCV.notify_one();
		}

		static void PushMainThreadJob(Job* job)
		{
			std::lock_guard<std::mutex> lock(m_mainThreadQueueMutex);
			m_mainThreadQueue.push_back(job);
		}

		void WorkerThread(unsigned int threadIndex)
		{
			m_threadIndex = threadIndex;
			m_started.wait(false);
			while (!m_stop)
			{
				Job* job = GetJob();
				RunJob(job);
			}
		}

		void RunJob(Job* job)
		{
			if (job == nullptr)
			{
				return;
			}
			m_runningJob = job;
			ASSERT(!job->Done());
			if (job)
			{
				job->Run();
			}
			m_runningJob = nullptr;
		}

		Job* GetJob()
		{
			std::unique_lock<std::mutex> lock(m_globalQueueMutex);
			m_globalCV.wait(lock, [this] { return m_stop || (!m_globalQueue.empty()); });
			if (m_stop)
			{
				return nullptr;
			}

			auto job = m_globalQueue.front();
			m_globalQueue.pop_front();
			return job;
		}
	};

	template<typename R>
	void Task<R>::CheckRescheduleAfterAwait(Job* child)
	{
		if (m_waitingFor.compare_exchange_strong(child, nullptr))
		{
			JobSystem::ReScheduleJob(this);
		}
	}

	template<typename Promise>
	constexpr void InitialTaskAwaiter::await_suspend(std::coroutine_handle<Promise> handle) const noexcept
	{
		JobSystem::ScheduleJob(handle.promise().task);
	}
}

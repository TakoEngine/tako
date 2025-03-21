module;
#include "Utility.hpp"
#include <vector>
#include <thread>
#include <atomic>
#include <optional>
#include <coroutine>
#include <queue>
#include <mutex>
export module Tako.JobSystem;

import Tako.Allocators.FreeListAllocator;
import Tako.Allocators.PoolAllocator;


namespace tako
{
	class JobSystem;

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
			LOG("Coroutine returned {}", value);
			result = std::move(value);
		}

		std::optional<R> result;
	};

	template<typename R>
	class Task;

	struct InitialTaskAwaiter
	{
		constexpr bool await_ready() const noexcept { return false; }

		template<typename Promise>
		constexpr void await_suspend(std::coroutine_handle<Promise> handle) const noexcept
		{
			//LOG("Schedule it! {}");
			JobSystem::Schedule(handle.promise().task);
		}

		constexpr void await_resume() const noexcept {}
	};

	template<typename R = void>
	struct Promise : public PromiseBase<R>
	{
		InitialTaskAwaiter initial_suspend() noexcept { LOG("initial suspend {}", (void*) task); return {}; }
		std::suspend_always final_suspend() noexcept
		{
			LOG("final suspend {}", (void*)task);
			if (task->m_isAwaitedOn && task->m_parent)
			{
				JobSystem::Schedule(task->m_parent);
			}
			return {};
		}

		Task<R> get_return_object()
		{
			LOG("get return");
			return Task<R>(std::coroutine_handle<Promise<R>>::from_promise(*this));
		}

		void unhandled_exception() {}

		Task<R>* task;
	};

	class Job
	{
	public:
		virtual bool Done() = 0;
		virtual void Resume() = 0;
	};

	export template<typename R = void>
	class Task : public Job
	{
	public:
		using promise_type = Promise<R>;
		friend promise_type;

		constexpr explicit Task(std::coroutine_handle<promise_type> handle) noexcept : m_handle(handle)
		{
			LOG("This! {}", (void*) this);
			m_parent = JobSystem::m_runningJob;
			handle.promise().task = this;
		}

		constexpr ~Task() noexcept
		{
			m_handle.destroy();
		}

		bool Done() override
		{
			return m_done;
		}

		void Resume() override
		{
			if (!m_done)
			{
				m_handle.resume();
				m_done = m_handle.done();
			}
		}

		auto operator co_await()
		{
			struct Awaiter
			{
				Task* task;

				bool await_ready() const noexcept { return task->m_done; }
				void await_suspend(std::coroutine_handle<> handle) const noexcept
				{
					// ...
					LOG("awaiter suspend {}", (void*) task);
					task->m_isAwaitedOn = true;
					//return task->m_done;
				}

				auto await_resume() const noexcept
				{
					LOG("awaiter resume {}", (void*)task);
					if constexpr (!std::is_void_v<R>)
					{
						return task->m_handle.promise().result.value();
					}
				}
			};

			return Awaiter{ this };
		}
	private:
		bool m_done = false;
		Job* m_parent = nullptr;
		std::atomic<bool> m_isAwaitedOn = false;
		std::coroutine_handle<promise_type> m_handle;
	};

	export class JobSystem
	{
		template<typename R>
		friend class Task;
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
			m_cv.notify_all();
		}

		static void Schedule(Job* job)
		{
			{
				std::lock_guard<std::mutex> lock(m_queueMutex);
				m_queue.push(job);
			}
			m_cv.notify_one();
		}


	private:
		std::vector<std::thread> m_workers;
		std::atomic<bool> m_stop = false;

		static inline thread_local unsigned int m_threadIndex;
		static inline unsigned int m_threadCount;
		static inline std::queue<Job*> m_queue;
		static inline std::mutex m_queueMutex;
		static inline std::condition_variable m_cv;
		static inline thread_local Job* m_runningJob = nullptr;

		void WorkerThread(unsigned int threadIndex)
		{
			m_threadIndex = threadIndex;
			while (!m_stop)
			{
				Job* job = nullptr;
				{
					std::unique_lock<std::mutex> lock(m_queueMutex);
					m_cv.wait(lock, [this] { return m_stop || (!m_queue.empty()); });
					if (m_stop)
					{
						return;
					}

					job = m_queue.front();
					m_queue.pop();
				}
				m_runningJob = job;
				if (job)
				{
					job->Resume();
				}
				m_runningJob = nullptr;
			}
		}
	};
}

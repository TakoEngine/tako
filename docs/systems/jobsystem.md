# JobSystem

The job system provides a light weight way to distribute tasks into multiple threads, utilizing [C++20 coroutines](https://en.cppreference.com/w/cpp/language/coroutines.html) to be able to write synchronous looking code.

## Usage

```cpp
/// Define a workload using Task<ReturnType>
Task<int> Fibonacci(int n)
{
    if (n <= 1)
    {
        co_return n;
    }

    // Calling a task automatically schedules it
    auto child1 = Fibonacci(n - 1);
    auto child2 = Fibonnaci(n - 2);

    // At this point, child1 and child2 are running in parallel
    // on available worker threads.

    // Use co_await to fetch results and ensure completion
    int result1 = co_await child1;
    int result2 = co_await child2;

    co_return result1 + result2;
}
```
You can convert existing functions or lambdas, without having to redefine them as tasks, using the `JobSystem::Taskify` helper

```cpp
// Existing synchronous function
int CalculateMeaningOfLife();

auto funcTask = JobSystem::Taskify(CalculateMeaningOfLife);
auto lambdaTask = JobSystem::Taskify([&]()
{
    // Expensive calculation
    return 42;
});
```

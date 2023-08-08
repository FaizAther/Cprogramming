#include <benchmark/benchmark.h>

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

/*
 * Spinlock (Naive)
 */
class Spinlock {
private:
	std::atomic<bool> locked{false};

public:
	void lock()
	{
		while (locked.exchange(true))
		{ ; }
	}

	void unlock() { locked.store(false); }
};

void inc(Spinlock& s, std::int64_t& val)
{
	for (int i = 0; i < 100000; ++i)
	{
		s.lock();
		val++;
		s.unlock();
	}
}

static void naive(benchmark::State &s)
{
	auto num_threads = s.range(0);

	std::int64_t val = 0;

	std::vector<std::thread> threads;
	threads.reserve(num_threads);

	Spinlock sl;

	for (auto _ : s)
	{
		for (auto i = 0u; i < num_threads; i++)
		{
			threads.emplace_back([&] { inc(sl, val); });
		}
		for (auto &thread : threads) thread.join();
		threads.clear();
	}
}

BENCHMARK(naive)
	->RangeMultiplier(2)
	->Range(1, std::thread::hardware_concurrency())
	->UseRealTime()
	->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

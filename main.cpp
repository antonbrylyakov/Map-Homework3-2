#include <iostream>
#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>
#include <future>
#include <atomic>
#include <random>

using namespace std::chrono;

// Параллельный for_each
template <typename TIt, typename TFunc>
void par_for_each(TIt first, TIt last, TFunc& func)
{
	static const int minSize = 10000;
	auto size = std::distance(first, last);
	if (size <= minSize)
	{
		std::for_each(first, last, func);
	}
	else
	{
		auto middle = first;
		std::advance(middle, size / 2);
		auto fut = std::async(par_for_each<TIt, TFunc>, middle, last, std::ref(func));
		par_for_each(first, middle, func);
		fut.wait();
	}
}


int main()
{
	setlocale(LC_ALL, "Russian");
#ifdef _WIN32
	SetConsoleCP(1251);
#endif

	std::mt19937 gen(steady_clock::now().time_since_epoch().count());
	std::uniform_int_distribution<long> dis(-100, 100);
	auto rand_num([&dis, &gen]() mutable { return dis(gen); });
	std::vector<long> vec(10'000'000);
	std::generate(vec.begin(), vec.end(), rand_num);

	{
		long sum = 0;

		auto start = steady_clock::now();
		std::for_each(vec.cbegin(), vec.cend(), [&sum](auto v) { sum += v; });
		auto end = steady_clock::now();
		std::chrono::duration<double> duration = end - start;

		std::cout << "Последовательное вычисление дает сумму: " << sum << ". Затрачено: " << duration.count() << " сек" << std::endl;
	}

	{
		std::atomic<long> sum(0);

		auto start = steady_clock::now();
		auto func = [&sum](auto v) { sum.fetch_add(v); };
		par_for_each(vec.cbegin(), vec.cend(), func);
		auto end = steady_clock::now();
		std::chrono::duration<double> duration = end - start;

		std::cout << "Рекурсивно-параллельное вычисление дает сумму: " << sum.load() << ". Затрачено: " << duration.count() << " сек" << std::endl;
	}
}
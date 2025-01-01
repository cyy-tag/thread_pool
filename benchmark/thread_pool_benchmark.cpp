#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <benchmark/benchmark.h>

#include "thread_pool.h"

//global test data
int global_data;

void TestInit(benchmark::State& state) {
  std::random_device rd_;
  std::mt19937 gen_{rd_()};
  std::uniform_int_distribution<> distrib(1, 1000);
  global_data = distrib(gen_);
}

bool is_prime(uint64_t n) noexcept {
    uint64_t flag = true;
    for(int i = 2;i<=n/i;i++){
        if(n%i==0){
            flag = false;
            break;
        }
    }
    return flag;
}

class ThreadPoolBenchMark : public ::benchmark::Fixture {
public:
  util::ThreadPool<> thread_pool{std::thread::hardware_concurrency()};
};

BENCHMARK_DEFINE_F(ThreadPoolBenchMark, BM_ThreadPool)(benchmark::State& state) {
  for (auto _ : state) {
    auto future = thread_pool.Post(is_prime, global_data);
    future.get();
  }
}

//multiple thread
BENCHMARK_REGISTER_F(ThreadPoolBenchMark, BM_ThreadPool)->ThreadRange(1, 8);

BENCHMARK_MAIN();

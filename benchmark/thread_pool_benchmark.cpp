#include <cstdint>
#include <iostream>
#include <random>
#include <vector>
#include <benchmark/benchmark.h>

#include "thread_pool.h"

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
  std::random_device rd_;
  std::mt19937 gen_{rd_()};
  uint32_t random_val_;
  util::ThreadPool<> thread_pool{std::thread::hardware_concurrency()};

  ThreadPoolBenchMark() {

  }

  void SetUp(const ::benchmark::State& state) override {
    std::uniform_int_distribution<> distrib(1, 100001);
    random_val_ = distrib(gen_);
  }

  void TearDown(const ::benchmark::State& state) override {
  }
};

BENCHMARK_DEFINE_F(ThreadPoolBenchMark, BM_ThreadPool)(benchmark::State& state) {
  for (auto _ : state) {
    std::promise<bool> promise;
    auto future = promise.get_future();
    thread_pool.Dispatch([&](){
      promise.set_value(is_prime(random_val_));
    });
    future.get();
  }
}

//multiple thread
BENCHMARK_REGISTER_F(ThreadPoolBenchMark, BM_ThreadPool)->ThreadRange(1, 8);

BENCHMARK_MAIN();

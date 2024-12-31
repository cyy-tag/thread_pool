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
  std::vector<uint32_t> values_;

  ThreadPoolBenchMark() {
    std::uniform_int_distribution<> distrib(1, 100001);
    for(int i = 0; i < 1000; ++i) {
      values_.emplace_back(distrib(gen_));
    }
  }

  void SetUp(const ::benchmark::State& state) override {

  }

  void TearDown(const ::benchmark::State& state) override {
  }
};

BENCHMARK_DEFINE_F(ThreadPoolBenchMark, BM_ThreadPool)(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<Future<bool>> futures;
    for(const auto& value : values_) {
      futures.emplace_back(std::move(thread_pool.Post(is_prime, value)));
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    for(auto&& future : futures) {
      if(future.valid()) {
        future.get();
      } else {
        printf("future not valid\n");
      }
    }
  }
}

//multiple thread
BENCHMARK_REGISTER_F(ThreadPoolBenchMark, BM_ThreadPool)->ThreadRange(1, 8);

BENCHMARK_MAIN();

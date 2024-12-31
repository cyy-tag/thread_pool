#include <vector>
#include <gtest/gtest.h>
#include "thread_pool.h"

class ThreadPoolTest : public ::testing::Test {
public:
  util::ThreadPool<> thread_pool_{std::thread::hardware_concurrency()};
protected:
  void SetUp() override {

  }

  void TearDown() override {

  }
};

TEST_F(ThreadPoolTest, TestExample) {
  for(int i = 0; i < 1000; ++i) {
    auto promise = std::promise<int>{};
    auto future = promise.get_future();
    thread_pool_.Dispatch([&]{
        promise.set_value(i);
      });
    EXPECT_EQ(i, future.get());
  }
}

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
    auto future = thread_pool_.Post([&]() noexcept {
        return i;
      });
    EXPECT_EQ(i, future.get());
  }
}

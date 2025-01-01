include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  thread_pool_test
  thread_pool_test.cpp
)
target_link_libraries(
  thread_pool_test
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(thread_pool_test)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
)

FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.1
)

FetchContent_MakeAvailable(googlebenchmark)

FetchContent_Declare(
  thread-pool
  GIT_REPOSITORY https://github.com/DeveloperPaul123/thread-pool.git
  GIT_TAG master
)

FetchContent_MakeAvailable(thread-pool)

add_executable(thread_pool_benchmark thread_pool_benchmark.cpp)
target_link_libraries(thread_pool_benchmark PRIVATE benchmark::benchmark)

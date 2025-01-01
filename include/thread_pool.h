#pragma once
#include <functional>
#include <future>
#include <thread>
#include <vector>

#include "notify_queue.h"

namespace util
{
template<typename Functor = std::move_only_function<void() noexcept>, 
         typename LockType = std::mutex,
         typename InitFunc = std::function<void()>>
requires is_lockable<LockType>
class ThreadPool {
public:
  explicit ThreadPool(size_t thread_num, InitFunc init = []() noexcept {}) 
  : count_(thread_num),
    index_(0),
    queues_(thread_num)
  {
    for(size_t i = 0; i < thread_num; ++i) {
      threads_.emplace_back([&, init=init, id = i](){
        init();
        while(true) {
          Functor f{nullptr};
          for(unsigned n = 0; n < count_ && !f; ++n) {
            queues_[(n + id) % count_].TryDequeue(f);
          }
          if(!f) {
            bool done = queues_[id].Dequeue(f);
            if(done) return;
          }
          if(f) f();
        }
      });
    }
  }

  ~ThreadPool()
  {
    for(auto&& q : queues_) {
      q.Done();
    }
    for(auto&& thread : threads_) {
      thread.join();
    }
  }

  template<typename F, typename... Args,
  typename = std::enable_if_t<std::is_nothrow_invocable_v<F, Args...>>>
  void PostDetach(F&& func, Args&&... args) noexcept {
    Dispatch([f = std::move(func),
                        ... largs = std::move(args)] noexcept {
                          std::invoke(f, largs...);
                        });
  }

  template<typename F, typename... Args, 
  typename R=std::enable_if_t<std::is_nothrow_invocable_v<F, Args...>, std::result_of_t<F&&(Args&&...)>>>
  std::future<R> Post(F&& func, Args&&... args) noexcept {
    std::promise<R> promise;
    auto future = promise.get_future();
    Dispatch([f = std::move(func),
              ... largs = std::move(args),
              p = std::move(promise)] mutable noexcept {
                if constexpr(std::is_same_v<R, void>) {
                  std::ignore = std::invoke(f, largs...);
                  p.set_value();
                } else {
                  R result = std::invoke(f, largs...);
                  p.set_value(std::move(result));
                }
              });
    return future;
  }

private:
  void Dispatch(Functor&& func) noexcept {
    auto i = index_++;

    for(unsigned n = 0; n < count_; ++n) {
      if(queues_[(i+n) % count_].TryEnqueue(std::forward<Functor>(func))) return;
    }
    queues_[i%count_].Enqueue(std::forward<Functor>(func));
  }

  unsigned count_;
  std::atomic<unsigned> index_;
  std::vector<NotifyQueue<Functor, LockType>> queues_;
  std::vector<std::jthread> threads_;
};
  
} // namespace util

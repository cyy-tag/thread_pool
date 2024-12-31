#pragma once
#include <functional>
#include <future>
#include <thread>
#include <vector>

#include "notify_queue.h"

namespace util
{
template<typename Functor = std::move_only_function<void()>, typename LockType = std::mutex>
requires is_lockable<LockType>
class ThreadPool {
public:
  ThreadPool(size_t thread_num) 
  : count_(thread_num),
    index_(0),
    queues_(thread_num)
  {
    for(size_t i = 0; i < thread_num; ++i) {
      threads_.emplace_back([&, id = i](){
        while(true) {
          Functor f{};
          for(unsigned n = 0; n < count_ && !f; ++n) {
            f = queues_[(n + id) % count_].TryDequeue();
          }
          if(!f) {
            bool done = false;
            std::tie(done, f) = queues_[id].Dequeue();
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

  // template<typename F, typename... Args>
  // void PostDetach(F&& func, Args&&... args) noexcept {
  //   Dispatch(std::move([f = std::move(func),
  //                       ... largs = std::move(args)]{
  //                         std::invoke(f, largs...);
  //                       }));
  // }

  // template<typename F, typename... Args, typename R=std::result_of_t<F&&(Args&&...)>>
  // std::future<R> Post(F&& func, Args&&... args) noexcept {
  //   std::promise<R> promise;
  //   auto future = promise.get_future();
  //   Dispatch([f = std::move(func),
  //             ... largs = std::move(args),
  //             p = std::move(promise)] mutable {
  //                 R result = std::invoke(f, largs...);
  //                 p.set_value(std::move(result));
  //             });
  //   return future;
  // }

  template<typename F>
  void Dispatch(F&& func) noexcept {
    auto i = index_++;

    for(unsigned n = 0; n < count_; ++n) {
      if(queues_[(i+n) % count_].TryEnqueue(std::forward<F>(func))) return;
    }
    queues_[i%count_].Enqueue(std::forward<F>(func));
  }

private:
  unsigned count_;
  std::atomic<unsigned> index_;
  std::vector<NotifyQueue<Functor, LockType>> queues_;
  std::vector<std::jthread> threads_;
};
  
} // namespace util

#pragma once
#include <functional>
#include <future>
#include <thread>
#include <vector>

#include "notify_queue.h"
#include "custom_promise.h"

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

  template<typename F, typename... Args>
  void PostDetach(F&& func, Args&&... args) noexcept {
    Dispatch(std::move([f = std::move(func),
                        ... largs = std::move(args)]{
                          std::invoke(f, largs...);
                        }));
  }

  template<typename F, typename... Args, typename R=std::result_of_t<F&&(Args&&...)>>
  Future<R> Post(F&& func, Args&&... args) noexcept {
    auto promise_ptr = std::make_shared<CustomPromise<R>>();
    int id = promise_ptr->ID();
    auto future = promise_ptr->get_future();
    Dispatch([f = std::move(func),
              ... largs = std::move(args),
              promise_ptr,
              id] {
                  R result = std::invoke(f, largs...);
                  if(!promise_ptr) {
                    printf("empty id %d\n", id);
                  } else {
                    promise_ptr->set_value(std::move(result));
                  }
              }, id, promise_ptr);
    return future;
  }

  template<typename F>
  void Dispatch(F&& func) noexcept {
    auto t_ptr = std::shared_ptr<int>();
    auto i = index_++;
    for(unsigned n = 0; n < count_; ++n) {
      if(queues_[(i+n) % count_].TryEnqueue(std::forward<F>(func), 1, t_ptr)) return;
    }
    queues_[i%count_].Enqueue(std::forward<F>(func), 1, t_ptr);
  }

  template<typename F, typename Ptr>
  void Dispatch(F&& func, int id, Ptr& ptr) noexcept {
    auto i = index_++;
    for(unsigned n = 0; n < count_; ++n) {
      if(queues_[(i+n) % count_].TryEnqueue(std::forward<F>(func), id, ptr)) return;
    }
    queues_[i%count_].Enqueue(std::forward<F>(func), id, ptr);
  }

private:
  unsigned count_;
  std::atomic<unsigned> index_;
  std::vector<NotifyQueue<Functor, LockType>> queues_;
  std::vector<std::jthread> threads_;
};
  
} // namespace util

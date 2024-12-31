#pragma once
#include <cassert>
#include <concepts>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace util {

template<typename LockType>
concept is_lockable = requires(LockType&& lock) {
  lock.lock();
  lock.unlock();
  { lock.try_lock() } -> std::convertible_to<bool>;
};

template<typename T, typename LockType>
requires is_lockable<LockType>
class NotifyQueue {
  public:
    using value_type = T;
    using size_type = typename std::queue<T>::size_type;
    using lock_t = std::unique_lock<LockType>;

    NotifyQueue()=default;
    NotifyQueue(const NotifyQueue&)=delete;
    NotifyQueue& operator&(const NotifyQueue&)=delete;

    template<typename Ptr>
    void Enqueue(T&& value, int id, Ptr& ptr) noexcept {
      {
        // printf("enqueue-id %d use_count %ld\n", id, ptr.use_count());
        lock_t lk{mutex_};
        q_.emplace(std::forward<T>(value));
      }
      // printf("enqueue id %d use_count %ld\n", id, ptr.use_count());
      cv_.notify_one();
    }

    template<typename Ptr>
    [[nodiscard]] bool TryEnqueue(T&& value, int id, Ptr& ptr) noexcept {
      {
        assert((ptr.use_count()) != 2 && "error use_count");
        lock_t lk{mutex_, std::try_to_lock};
        if(!lk) return false;
        q_.emplace(std::forward<T>(value));
      }
      cv_.notify_one();
      return true;
    }

    [[nodiscard]] std::pair<bool, T> Dequeue() noexcept {
      lock_t lk{mutex_};
      while(q_.empty() && !done_) {
        cv_.wait(lk);
      }
      if(q_.empty()) return {done_, nullptr};
      auto result = std::move(q_.front());
      q_.pop();
      return {false, std::move(result)};
    }

    [[nodiscard]] T TryDequeue() noexcept {
      lock_t lk{mutex_, std::try_to_lock};
      if(!lk || q_.empty()) return nullptr;
      auto result = std::move(q_.front());
      q_.pop();
      return result;
    }

    [[nodiscard]] size_type Size() const noexcept {
      lock_t lk{mutex_};
      return q_.size();
    }

    void Done() noexcept {
      {
        lock_t lk{mutex_};
        done_ = true;
      }
      cv_.notify_one();
    }

  private:
    std::queue<T> q_;
    std::condition_variable cv_;
    LockType mutex_;
    bool done_{false};
};

} // namespace util

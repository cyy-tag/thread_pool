#pragma once
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

    void Enqueue(T&& value) noexcept {
      {
        lock_t lk{mutex_};
        q_.emplace(std::forward<T>(value));
      }
      cv_.notify_one();
    }

    [[nodiscard]] bool TryEnqueue(T&& value) noexcept {
      {
        lock_t lk{mutex_, std::try_to_lock};
        if(!lk) return false;
        q_.emplace(std::forward<T>(value));
      }
      cv_.notify_one();
      return true;
    }

    [[nodiscard]] bool Dequeue(T& value) noexcept {
      lock_t lk{mutex_};
      while(q_.empty() && !done_) {
        cv_.wait(lk);
      }
      if(q_.empty()) return true;
      value = std::move(q_.front());
      q_.pop();
      return false;
    }

    void TryDequeue(T& value) noexcept {
      lock_t lk{mutex_, std::try_to_lock};
      if(!lk || q_.empty()) return;
      value = std::move(q_.front());
      q_.pop();
      return;
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

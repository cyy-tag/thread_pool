#pragma once
#include <atomic>
#include <iostream>
#include <stacktrace>
#include <future>
#include <memory>

template<typename Res>
class Future
{
public:
  Future(int id, std::future<Res> future)
  :id_(id), future_(std::move(future))
  {
    // printf("future-id ctor %d %p\n", id_, (void*)this);
  }

  Future(Future&& rhs) 
  :id_(rhs.id_), future_(std::move(rhs.future_))
  {
    // printf("future-id %d %p move %p\n", id_, (void*)&rhs, (void*)this);
  }

  ~Future() {
    // printf("future-id %d dector \n", id_);
  }

  Res get() {
    // printf("future-id %d get %p\n", id_, (void*)this);
    return future_.get();
  }

  bool valid() {
    return future_.valid();
  }

private:
  int id_;
  std::future<Res> future_;
};

template<typename Res>
class CustomPromise
{
public:
  CustomPromise()
  {
    self_id = ++global_id_;
    promise_ptr = new std::promise<Res>();
    printf("promise-id %d ctor addr %p\n", self_id, (void*)this);
  }

  ~CustomPromise() {
    if(promise_ptr) {
        std::stacktrace st = std::stacktrace::current();
        printf("stacktrace %p delete id %d %s\n", (void*)this, self_id, std::to_string(st).c_str());
        delete promise_ptr;
    }
  }

  CustomPromise(const CustomPromise&) = delete;
  CustomPromise& operator=(const CustomPromise&) = delete;

  CustomPromise(CustomPromise&& rhs)
  {
    self_id = rhs.self_id;
    promise_ptr = rhs.promise_ptr;
    rhs.promise_ptr = nullptr;
    printf("promise-id %d move ctor %p to %p\n", self_id, (void*)&rhs, (void*)this);
  }

  CustomPromise& operator=(CustomPromise&& rhs) {
    if(this != & rhs)  {
      promise_ptr = rhs.promise_ptr;
      rhs.promise_ptr = nullptr;
      self_id = rhs.self_id;
      printf("promise-id %d move assigned %p to %p\n", self_id, (void*)&rhs, (void*)this);
    }
    return *this;
  }

  Future<Res> get_future() {
    printf("promise-id %d get future, %p\n", self_id, (void*)this);
    return Future<Res>(self_id, std::move(promise_ptr->get_future()));
  }

  void set_value(Res&& value) {
    printf("promise-id %d set value %p\n", self_id, (void*)this);
    promise_ptr->set_value(std::forward<Res>(value));
  }

  int ID() { return self_id; }

private:
  static std::atomic<int> global_id_;
  int self_id{0};
  std::promise<Res>* promise_ptr{nullptr};
};


template<typename Res>
std::atomic<int> CustomPromise<Res>::global_id_ = 0;

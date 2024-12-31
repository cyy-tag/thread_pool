#pragma once
#include <cassert>
#include <iostream>
#include <mutex>
#include <atomic>

template<typename T>
struct share_data
{
    share_data(T* value, int id): value_(value), id_(id) {}
    void add() {
        assert(count_ > 0);
        ++count_;
    }

    void sub() {
        if(--count_ == 0) {
            printf("delete ptr %id\n", id_);
            delete value_;
        }
    }

    int id_{-1};
    int count_{1};
    T* value_{nullptr};
    std::mutex mutex_;
};


template<typename T>
class CustomPtr
{
public:
    CustomPtr(): data_(new share_data<T>(nullptr, -1)) {}
    CustomPtr(T* value, int id): data_(new share_data<T>(value, id)) {
        printf("ctor ptr id %d\n", id);
    }

    CustomPtr(const CustomPtr& rhs) {
        if(this != &rhs) {
            {
                std::lock_guard lk(rhs.data_->mutex_);
                rhs.data_->add();
                printf("copy ptr %id\n", rhs.data_->id_);
            }
            if(this->data_) {
                std::lock_guard lk(this->data_->mutex_);
                printf("copy remove ptr id %d\n", this->data_->id_);
                this->data_->sub();
            }
            this->data_ = rhs.data_;
        }
    }

    CustomPtr(CustomPtr&& rhs) {
        share_data<T>* data = rhs.data_;
        rhs.data_ = nullptr;
        if(this->data_) {
            std::lock_guard lk(this->data_->mutex_);
            printf("subd move move id %d\n", this->data_->id_);
            this->data_->sub();
        }
        this->data_ = data;
    }

    ~CustomPtr() {
        if(data_) {
            data_->sub();
        }
    }

    T* operator->() const {
        return data_->value_;
    }

    explicit operator bool() const noexcept {
        return data_ != nullptr;
    }

    int use_count() { return data_->count_; }
private:
    share_data<T>* data_;
};

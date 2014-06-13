// Author: Ivan Kotenkov <ivan.kotenkov@gmail.com>

#ifndef SHARED_POINTER_H_
#define SHARED_POINTER_H_

#include <cassert>

template <typename T>
class SharedPointer {
 public:
  explicit SharedPointer(T* data);
  SharedPointer(const SharedPointer& other);
  ~SharedPointer();

  SharedPointer& operator=(const SharedPointer& other);

  T* get() const { return ref_counter_->data(); }
  void reset(T* data);

 private:
  class RefCounter {
   public:
    explicit RefCounter(T* data);
    ~RefCounter();

    void Ref() const;
    void Deref() const;
    T* data() const { return data_; }

   private:
    T* const data_;
    mutable int count_;

    friend class SharedPointerTest;
  };

  const RefCounter* ref_counter_;

  friend class SharedPointerTest;
};

template <typename T>
SharedPointer<T>::RefCounter::RefCounter(T* data)
    : data_(data), count_(1) {
  assert(data_);
}

template <typename T>
SharedPointer<T>::RefCounter::~RefCounter() {
  assert(!count_);
  delete data_;
}

template <typename T>
void SharedPointer<T>::RefCounter::Ref() const {
  assert(count_ > 0);
  ++count_;
}

template <typename T>
void SharedPointer<T>::RefCounter::Deref() const {
  assert(count_ > 0);
  --count_;
  if (!count_)
    delete this;
}

template <typename T>
SharedPointer<T>::SharedPointer(T* data)
    : ref_counter_(data ? new RefCounter(data) : 0) {
}

template <typename T>
SharedPointer<T>::SharedPointer(const SharedPointer<T>& other)
    : ref_counter_(other.ref_counter_) {
  if (ref_counter_) {
    ref_counter_->Ref();
  }
}

template <typename T>
SharedPointer<T>::~SharedPointer() {
  if (ref_counter_) {
    ref_counter_->Deref();
  }
}

template <typename T>
SharedPointer<T>& SharedPointer<T>::operator=(const SharedPointer<T>& other) {
  if (this == &other || ref_counter_ == other.ref_counter_)
    return *this;

  if (ref_counter_)
    ref_counter_->Deref();

  ref_counter_ = other.ref_counter_;
  if (ref_counter_)
    ref_counter_->Ref();
}

template <typename T>
void SharedPointer<T>::reset(T* data) {
  if (ref_counter_)
    ref_counter_->Deref();
  ref_counter_ = data ? new RefCounter(data) : 0;
}

#endif  // SHARED_POINTER_H_

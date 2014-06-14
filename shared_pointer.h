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

  T* Get() const { return ref_counter_->data(); }
  void Reset(T* data);

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

  void Ref() const;
  void Deref() const;

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
  Ref();
}

template <typename T>
SharedPointer<T>::~SharedPointer() {
  Deref();
}

template <typename T>
SharedPointer<T>& SharedPointer<T>::operator=(const SharedPointer<T>& other) {
  if (this == &other || ref_counter_ == other.ref_counter_)
    return *this;

  Deref();
  ref_counter_ = other.ref_counter_;
  Ref();
}

template <typename T>
void SharedPointer<T>::Reset(T* data) {
  Deref();
  ref_counter_ = data ? new RefCounter(data) : 0;
}

template <typename T>
void SharedPointer<T>::Ref() const {
  if (ref_counter_)
    ref_counter_->Ref();
}

template <typename T>
void SharedPointer<T>::Deref() const {
  if (ref_counter_)
    ref_counter_->Deref();
}
#endif  // SHARED_POINTER_H_

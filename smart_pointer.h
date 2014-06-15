// Author: Ivan Kotenkov <ivan.kotenkov@gmail.com>

#ifndef SMART_POINTER_H_
#define SMART_POINTER_H_

#include <cassert>

namespace memory_details {
class SmartPointerTest;

template <typename T>
class RefCounter {
 public:
  explicit RefCounter(T* data);
  ~RefCounter();

  void StrongRef();
  void StrongDeref();
  void WeakRef();
  void WeakDeref();

  T* data() const { return data_; }

 private:
  T* data_;
  int strong_count_;
  int weak_count_;

  friend class SmartPointerTest;
};

template <typename T>
RefCounter<T>::RefCounter(T* data)
    : data_(data), strong_count_(1), weak_count_(1) {
  assert(data_);
}

template <typename T>
RefCounter<T>::~RefCounter() {
  assert(!strong_count_);
  assert(!weak_count_);
  assert(!data_);
}

template <typename T>
void RefCounter<T>::StrongRef() {
  assert(strong_count_ > 0);
  ++strong_count_;
}

template <typename T>
void RefCounter<T>::StrongDeref() {
  assert(strong_count_ > 0);
  --strong_count_;
  if (!strong_count_) {
    delete data_;
    data_ = 0;
  }
}

template <typename T>
void RefCounter<T>::WeakRef() {
  assert(weak_count_ > 0);
  ++weak_count_;
}

template <typename T>
void RefCounter<T>::WeakDeref() {
  assert(weak_count_ > 0);
  --weak_count_;
  if (!weak_count_)
    delete this;
}

template <typename T>
class SmartPointerBase {
 public:
  explicit SmartPointerBase(T* data);
  SmartPointerBase(const SmartPointerBase& other);
  virtual ~SmartPointerBase() { }

  SmartPointerBase& operator=(const SmartPointerBase& other);

  void Reset(T* data);

 protected:
  virtual void Ref() = 0;
  virtual void Deref() = 0;

  memory_details::RefCounter<T>* ref_counter_;

  friend class memory_details::SmartPointerTest;
};

template <typename T>
SmartPointerBase<T>::SmartPointerBase(T* data)
    : ref_counter_(data ? new memory_details::RefCounter<T>(data) : 0) {
}

template <typename T>
SmartPointerBase<T>::SmartPointerBase(const SmartPointerBase<T>& other)
    : ref_counter_(other.ref_counter_ && other.ref_counter_->data()
          ? other.ref_counter_ : 0) {
}

template <typename T>
SmartPointerBase<T>& SmartPointerBase<T>::operator=(const SmartPointerBase<T>& other) {
  if (this == &other || ref_counter_ == other.ref_counter_)
    return *this;

  Deref();
  ref_counter_ = other.ref_counter_ && other.ref_counter_->data()
      ? other.ref_counter_ : 0;
  Ref();
  return *this;
}

template <typename T>
void SmartPointerBase<T>::Reset(T* data) {
  Deref();
  ref_counter_ = data ? new memory_details::RefCounter<T>(data) : 0;
}
}  // namespace memory_details

template <typename T>
class WeakPointer;

template <typename T>
class SharedPointer : public memory_details::SmartPointerBase<T> {
 public:
  explicit SharedPointer(T* data)
      : memory_details::SmartPointerBase<T>(data) { }
  SharedPointer(const SharedPointer& other);
  virtual ~SharedPointer();

  T* Get() const { return this->ref_counter_->data(); }

 private:
  explicit SharedPointer(const WeakPointer<T>& other);
  virtual void Ref();
  virtual void Deref();

  friend class WeakPointer<T>;
};

template <typename T>
SharedPointer<T>::SharedPointer(const SharedPointer<T>& other)
    : memory_details::SmartPointerBase<T>(other) {
  Ref();
}

template <typename T>
SharedPointer<T>::~SharedPointer() {
  Deref();
}

template <typename T>
SharedPointer<T>::SharedPointer(const WeakPointer<T>& other)
    : memory_details::SmartPointerBase<T>(other) {
  Ref();
}

template <typename T>
void SharedPointer<T>::Ref() {
  if (this->ref_counter_) {
    this->ref_counter_->WeakRef();
    this->ref_counter_->StrongRef();
  }
}

template <typename T>
void SharedPointer<T>::Deref() {
  if (this->ref_counter_) {
    this->ref_counter_->StrongDeref();
    this->ref_counter_->WeakDeref();
  }
}

template <typename T>
class WeakPointer : public memory_details::SmartPointerBase<T> {
 public:
  explicit WeakPointer(const SharedPointer<T>& other);
  WeakPointer(const WeakPointer<T>& other);
  virtual ~WeakPointer();

  SharedPointer<T> lock();

 private:
  virtual void Ref();
  virtual void Deref();
  void Reset() { }
};

template <typename T>
WeakPointer<T>::WeakPointer(const SharedPointer<T>& other)
    : memory_details::SmartPointerBase<T>(other) {
  Ref();
}

template <typename T>
WeakPointer<T>::WeakPointer(const WeakPointer<T>& other)
    : memory_details::SmartPointerBase<T>(other) {
  Ref();
}

template <typename T>
WeakPointer<T>::~WeakPointer() {
  Deref();
}

template <typename T>
SharedPointer<T> WeakPointer<T>::lock() {
  if (this->ref_counter_ && !this->ref_counter_->data()) {
    Deref();
    this->ref_counter_ = 0;
  }
  return SharedPointer<T>(*this);
}

template <typename T>
void WeakPointer<T>::Ref() {
  if (this->ref_counter_)
    this->ref_counter_->WeakRef();
}

template <typename T>
void WeakPointer<T>::Deref() {
  if (this->ref_counter_)
    this->ref_counter_->WeakDeref();
}

#endif  // SMART_POINTER_H_

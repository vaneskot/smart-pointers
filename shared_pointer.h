// Author: Ivan Kotenkov <ivan.kotenkov@gmail.com>

#ifndef SHARED_POINTER_H_
#define SHARED_POINTER_H_

#include <cassert>
namespace memory_details {
class SharedPointerTest;
template <typename T>
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

template <typename T>
RefCounter<T>::RefCounter(T* data)
    : data_(data), count_(1) {
  assert(data_);
}

template <typename T>
RefCounter<T>::~RefCounter() {
  assert(!count_);
  delete data_;
}

template <typename T>
void RefCounter<T>::Ref() const {
  assert(count_ > 0);
  ++count_;
}

template <typename T>
void RefCounter<T>::Deref() const {
  assert(count_ > 0);
  --count_;
  if (!count_)
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
  virtual void Ref() const = 0;
  virtual void Deref() const = 0;

  const memory_details::RefCounter<T>* ref_counter_;

  friend class memory_details::SharedPointerTest;
};

template <typename T>
SmartPointerBase<T>::SmartPointerBase(T* data)
    : ref_counter_(data ? new memory_details::RefCounter<T>(data) : 0) {
}

template <typename T>
SmartPointerBase<T>::SmartPointerBase(const SmartPointerBase<T>& other)
    : ref_counter_(other.ref_counter_) {
}

template <typename T>
SmartPointerBase<T>& SmartPointerBase<T>::operator=(const SmartPointerBase<T>& other) {
  if (this == &other || ref_counter_ == other.ref_counter_)
    return *this;

  Deref();
  ref_counter_ = other.ref_counter_;
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
class SharedPointer : public memory_details::SmartPointerBase<T> {
 public:
  explicit SharedPointer(T* data)
      : memory_details::SmartPointerBase<T>(data) { }
  SharedPointer(const SharedPointer& other);
  virtual ~SharedPointer();

  T* Get() const { return this->ref_counter_->data(); }

 private:
  virtual void Ref() const;
  virtual void Deref() const;
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
void SharedPointer<T>::Ref() const {
  if (this->ref_counter_)
    this->ref_counter_->Ref();
}

template <typename T>
void SharedPointer<T>::Deref() const {
  if (this->ref_counter_)
    this->ref_counter_->Deref();
}
#endif  // SHARED_POINTER_H_

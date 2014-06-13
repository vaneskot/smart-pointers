// Author: Ivan Kotenkov <ivan.kotenkov@gmail.com>

#ifndef SHARED_POINTER_H_
#define SHARED_POINTER_H_

#include <assert.h>

template <typename T>
class SharedPointer {
 public:
  explicit SharedPointer(T* data);
  SharedPointer(const SharedPointer& other);
  ~SharedPointer();

  SharedPointer& operator=(const SharedPointer& other);

  T* get() const { return ref_->data(); }
  void reset(T* data);

 private:
  class Ref {
   public:
    explicit Ref(T* data);
    ~Ref();

    void ref() const;
    void deref() const;
    T* data() const { return data_; }

   private:
    T* const data_;
    mutable int count_;

    friend class SharedPointerTest;
  };

  const Ref* ref_;

  friend class SharedPointerTest;
};

template <typename T>
SharedPointer<T>::Ref::Ref(T* data)
    : data_(data), count_(1) {
  assert(data_);
}

template <typename T>
SharedPointer<T>::Ref::~Ref() {
  assert(!count_);
  delete data_;
}

template <typename T>
void SharedPointer<T>::Ref::ref() const {
  assert(count_ > 0);
  ++count_;
}

template <typename T>
void SharedPointer<T>::Ref::deref() const {
  assert(count_ > 0);
  --count_;
  if (!count_)
    delete this;
}

template <typename T>
SharedPointer<T>::SharedPointer(T* data)
    : ref_(data ? new Ref(data) : 0) {
}

template <typename T>
SharedPointer<T>::SharedPointer(const SharedPointer<T>& other)
    : ref_(other.ref_) {
  if (ref_) {
    ref_->ref();
  }
}

template <typename T>
SharedPointer<T>::~SharedPointer() {
  if (ref_) {
    ref_->deref();
  }
}

template <typename T>
SharedPointer<T>& SharedPointer<T>::operator=(const SharedPointer<T>& other) {
  if (this == &other || ref_ == other.ref_)
    return *this;

  if (ref_)
    ref_->deref();

  ref_ = other.ref_;
  if (ref_)
    ref_->ref();
}

template <typename T>
void SharedPointer<T>::reset(T* data) {
  if (ref_)
    ref_->deref();
  ref_ = data ? new Ref(data) : 0;
}

#endif  // SHARED_POINTER_H_

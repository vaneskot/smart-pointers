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

  T* get() const { return m_ref->data; }
  void reset(T* data);

 private:
  class Ref {
   public:
    explicit Ref(T* i_data);
    ~Ref();

    void ref();
    void deref();

    T* data;

   private:
    int count;
  };

  mutable Ref* m_ref;
};

template <typename T>
SharedPointer<T>::Ref::Ref(T* i_data)
    : data(i_data), count(1) {
  assert(data);
}

template <typename T>
SharedPointer<T>::Ref::~Ref() {
  assert(!count);
  assert(data);
  delete data;
}

template <typename T>
void SharedPointer<T>::Ref::ref() {
  assert(count > 0);
  assert(data);
  ++count;
}

template <typename T>
void SharedPointer<T>::Ref::deref() {
  assert(count > 0);
  assert(data);
  --count;
  if (!count)
    delete this;
}

template <typename T>
SharedPointer<T>::SharedPointer(T* data)
    : m_ref(data ? new Ref(data) : 0) {
}

template <typename T>
SharedPointer<T>::SharedPointer(const SharedPointer<T>& other)
    : m_ref(other.m_ref) {
  if (m_ref) {
    m_ref->ref();
  }
}

template <typename T>
SharedPointer<T>::~SharedPointer() {
  if (m_ref) {
    m_ref->deref();
  }
}

template <typename T>
SharedPointer<T>& SharedPointer<T>::operator=(const SharedPointer<T>& other) {
  if (this == &other || m_ref == other.m_ref)
    return *this;

  if (m_ref)
    m_ref->deref();

  m_ref = other.m_ref;
  m_ref->ref();
}

template <typename T>
void SharedPointer<T>::reset(T* data) {
  if (m_ref)
    m_ref->deref();
  m_ref = data ? new Ref(data) : 0;
}

#endif  // SHARED_POINTER_H_

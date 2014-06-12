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

  T* get() const { return m_ref->data(); }
  void reset(T* data);

 private:
  class Ref {
   public:
    explicit Ref(T* data);
    ~Ref();

    void ref() const;
    void deref() const;
    T* data() const { return m_data; }

   private:
    T* const m_data;
    mutable int m_count;
  };

  const Ref* m_ref;
};

template <typename T>
SharedPointer<T>::Ref::Ref(T* data)
    : m_data(data), m_count(1) {
  assert(m_data);
}

template <typename T>
SharedPointer<T>::Ref::~Ref() {
  assert(!m_count);
  delete m_data;
}

template <typename T>
void SharedPointer<T>::Ref::ref() const {
  assert(m_count > 0);
  ++m_count;
}

template <typename T>
void SharedPointer<T>::Ref::deref() const {
  assert(m_count > 0);
  --m_count;
  if (!m_count)
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

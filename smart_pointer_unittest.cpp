// Author: Ivan Kotenkov <ivan.kotenkov@gmail.com>

#include "smart_pointer.h"

#include <cassert>
#include <map>

namespace {

class DeleteChecker {
  // Class that keeps tracks of all allocations and deallocations made using
  // overloaded operator new and checks following invariants for each pointer:
  // - there were no two consecutive allocations without deallocation in between
  // them for one pointer;
  // - there were no two consecutive deallocations without allocation in between
  // them for one pointer;
  // - each allocated pointer was deallocated.
 public:
  static bool g_enabled;

  static DeleteChecker& Instance();
  ~DeleteChecker();
  void Alloc(void* p);
  void Dealloc(void* p);

 private:
  typedef std::map<void*, int> MemoryMap;

  DeleteChecker() : inside_checker_(false) {}

  MemoryMap alloc_map_;
  bool inside_checker_;
};

bool DeleteChecker::g_enabled = true;

DeleteChecker& DeleteChecker::Instance() {
  static DeleteChecker instance;
  return instance;
}

DeleteChecker::~DeleteChecker() {
  g_enabled = false;
  for (MemoryMap::iterator it = alloc_map_.begin();
      it != alloc_map_.end(); ++it) {
    assert(!it->second);
  }
}

void DeleteChecker::Alloc(void* p) {
  if (!inside_checker_ && g_enabled) {
    inside_checker_ = true;
    ++alloc_map_[p];
    assert(alloc_map_[p] == 1);
    inside_checker_ = false;
  }
}

void DeleteChecker::Dealloc(void* p) {
  if (!inside_checker_ && g_enabled) {
    inside_checker_ = true;
    assert(alloc_map_[p] > 0);
    --alloc_map_[p];
    inside_checker_ = false;
  }
}
} // namespace

// Overloaded operator new and delete notify DeleteChecker.
void* operator new(unsigned long size) throw(std::bad_alloc) {
  void* p = malloc(size);
  if (!p)
    throw std::bad_alloc();
  DeleteChecker::Instance().Alloc(p);
  return p;
}

void operator delete(void* p) throw() {
  DeleteChecker::Instance().Dealloc(p);
  free(p);
}

namespace memory_details {
class SmartPointerTest {
 public:
  void TestSharedPointer();
  void TestWeakPointer();

  void RunAllTests();
};

void SmartPointerTest::TestSharedPointer() {
  int* p_int = new int;
  SharedPointer<int> p(p_int);

  assert(p.ref_counter_);
  assert(p.ref_counter_->data_ == p_int);
  assert(p.ref_counter_->strong_count_ == 1);
  assert(p.ref_counter_->weak_count_ == 1);
  assert(p_int == p.Get());

  {
    SharedPointer<int> p1(p);
    assert(p1.ref_counter_);
    assert(p1.ref_counter_ == p.ref_counter_);
    assert(p.ref_counter_->data_ == p_int);
    assert(p.ref_counter_->strong_count_ == 2);
    assert(p.ref_counter_->weak_count_ == 2);
  }

  assert(p.ref_counter_->strong_count_ == 1);

  SharedPointer<int> p2(p);
  assert(p2.ref_counter_);
  assert(p2.ref_counter_ == p.ref_counter_);
  assert(p.ref_counter_->data_ == p_int);
  assert(p.ref_counter_->strong_count_ == 2);
  assert(p.ref_counter_->weak_count_ == 2);

  p2.Reset(0);
  assert(!p2.ref_counter_);
  assert(p.ref_counter_->data_ == p_int);
  assert(p.ref_counter_->strong_count_ == 1);
  assert(p.ref_counter_->weak_count_ == 1);

  p2 = p;
  assert(p2.ref_counter_);
  assert(p2.ref_counter_ == p.ref_counter_);
  assert(p.ref_counter_->data_ == p_int);
  assert(p.ref_counter_->strong_count_ == 2);
  assert(p.ref_counter_->weak_count_ == 2);
}

void SmartPointerTest::TestWeakPointer() {
  int* p_int = new int;
  SharedPointer<int> p(p_int);

  assert(p.ref_counter_);
  assert(p.ref_counter_->data_ == p_int);
  assert(p.ref_counter_->strong_count_ == 1);
  assert(p.ref_counter_->weak_count_ == 1);
  assert(p_int == p.Get());

  WeakPointer<int> wp(p);
  assert(wp.ref_counter_);
  assert(p.ref_counter_ == wp.ref_counter_);
  assert(p.ref_counter_->strong_count_ == 1);
  assert(p.ref_counter_->weak_count_ == 2);
  assert(p_int == p.Get());

  SharedPointer<int> p1(wp.lock());
  assert(p.ref_counter_ == p1.ref_counter_);
  assert(p.ref_counter_->strong_count_ == 2);
  assert(p.ref_counter_->weak_count_ == 3);
  assert(p_int == p.Get());

  {
    WeakPointer<int> wp1(wp);
    assert(p.ref_counter_ == wp1.ref_counter_);
    assert(p.ref_counter_->strong_count_ == 2);
    assert(p.ref_counter_->weak_count_ == 4);
    assert(p_int == p.Get());
  }

  assert(p.ref_counter_->strong_count_ == 2);
  assert(p.ref_counter_->weak_count_ == 3);
  assert(p_int == p.Get());

  WeakPointer<int> wp1(wp);
  assert(p.ref_counter_ == wp1.ref_counter_);
  assert(p.ref_counter_->strong_count_ == 2);
  assert(p.ref_counter_->weak_count_ == 4);
  assert(p_int == p.Get());

  p.Reset(0);
  p1.Reset(0);
  assert(!p.ref_counter_ && !p1.ref_counter_);
  assert(wp.ref_counter_);
  assert(wp.ref_counter_->strong_count_ == 0);
  assert(wp.ref_counter_->weak_count_ == 2);
  assert(wp.ref_counter_->data_ == 0);

  p = wp1.lock();

  assert(p.ref_counter_ == 0);
  assert(wp1.ref_counter_ == 0);
  assert(wp.ref_counter_);
  assert(wp.ref_counter_->strong_count_ == 0);
  assert(wp.ref_counter_->weak_count_ == 1);
}

void SmartPointerTest::RunAllTests() {
  TestSharedPointer();
  TestWeakPointer();
}
} // namespace memory_details

int main() {
  memory_details::SmartPointerTest test;
  test.RunAllTests();
  return 0;
}

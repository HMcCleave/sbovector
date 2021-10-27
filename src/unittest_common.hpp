#ifndef UNITTEST_COMMON_HPP
#define UNITTEST_COMMON_HPP

#include <gtest/gtest.h>

#include "sbovector.hpp"

#include <memory>

constexpr size_t SMALL_SIZE = 5;
constexpr size_t LARGE_SIZE = 100;
constexpr size_t SBO_SIZE = 16;

static_assert(SMALL_SIZE < SBO_SIZE);
static_assert(SBO_SIZE < LARGE_SIZE);

template <typename T>
struct CountingAllocator {
  struct Totals {
    int allocs_{0};
    int frees_{0};
  } * totals_;
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;

  CountingAllocator(Totals* totals) : totals_(totals) {}

  pointer allocate(size_t n, const void*) { return allocate(n); }
  pointer allocate(size_t n) {
    ++totals_->allocs_;
    return new T[n];
  }
  void deallocate(pointer p, size_t n) {
    ++totals_->frees_;
    delete[] p;
  }
};

template <typename T>
struct CustomAllocator {
  int dummy_;
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;

  pointer allocate(size_t n, const void*) { return allocate(n); }
  pointer allocate(size_t n) { return new T[n]; }
  void deallocate(pointer p, size_t) { delete[] p; }
};

using Trivial = int;
class NonTrivial {
 private:
  std::unique_ptr<int> val_;

 public:
  NonTrivial() { val_ = std::make_unique<int>(42); }

  ~NonTrivial() {}

  NonTrivial(const NonTrivial&) : NonTrivial() {}

  NonTrivial(NonTrivial&&) noexcept : NonTrivial() {}

  NonTrivial& operator=(const NonTrivial&) { return *this; }

  NonTrivial& operator=(NonTrivial&&) noexcept { return *this; }
};

class NoMove {
 public:
  NoMove() {}
  ~NoMove() {}
  NoMove(const NoMove&) {}
  NoMove(NoMove&&) = delete;
  NoMove& operator=(const NoMove&) { return *this; }
  NoMove& operator=(NoMove&&) = delete;
};

class NoCopy {
 public:
  NoCopy() {}
  ~NoCopy() {}
  NoCopy(const NoCopy&) = delete;
  NoCopy(NoCopy&&) noexcept {}
  NoCopy& operator=(const NoCopy&) = delete;
  NoCopy& operator=(NoCopy&&) noexcept {}
};

struct OperationCounter {
  struct OperationTotals {
    int default_constructor_{0};
    int copy_constructor_{0};
    int move_constructor_{0};
    int copy_assignment_{0};
    int move_assignment_{0};
    int moved_destructor_{0};
    int unmoved_destructor_{0};
    void reset() { memset(this, 0, sizeof(this)); }
    int moves() const { return move_constructor_ + move_assignment_; }
    int copies() const { return copy_constructor_ + copy_assignment_; }
    int constructs() const {
      return default_constructor_ + copy_constructor_ + move_constructor_;
    }
    int destructs() const { return moved_destructor_ + unmoved_destructor_; }
  };
  bool moved_{false};
  inline static OperationTotals TOTALS{};
  OperationCounter() {
    ++TOTALS.default_constructor_;
    moved_ = false;
  }
  OperationCounter(OperationCounter&& from) noexcept {
    ++TOTALS.move_constructor_;
    moved_ = false;
    from.moved_ = true;
  }
  OperationCounter(const OperationCounter&) { ++TOTALS.copy_constructor_; }
  OperationCounter& operator=(OperationCounter&& from) noexcept {
    ++TOTALS.move_assignment_;
    from.moved_ = true;
    return *this;
  }
  OperationCounter& operator=(const OperationCounter&) {
    ++TOTALS.copy_assignment_;
    return *this;
  }
  ~OperationCounter() {
    ++(moved_ ? TOTALS.moved_destructor_ : TOTALS.unmoved_destructor_);
  }
};

template <typename Data, typename Alloc = std::allocator<Data>>
struct TypeHelper {
  typedef typename Data DataType;
  typedef typename Alloc AllocatorType;
};

template <typename T>
struct SBOVector_ : public ::testing::Test {
  using DataType = typename T::DataType;
  using AllocatorType = typename T::AllocatorType;
  using ContainerType = SBOVector<DataType, SBO_SIZE, AllocatorType>;
};

struct DataTypeOperationTrackingSBOVector : public ::testing::Test {
  using DataType = OperationCounter;
  using AllocatorType = CountingAllocator<DataType>;
  using ContainerType = SBOVector<DataType, SBO_SIZE, AllocatorType>;
  AllocatorType::Totals totals_;
  AllocatorType create_allocator() { return AllocatorType(&totals_); }

  void SetUp() {
    memset(&totals_, 0, sizeof(totals_));
    OperationCounter::TOTALS.reset();
  }
};

typedef ::testing::Types<TypeHelper<Trivial>,
                         TypeHelper<Trivial, CustomAllocator<Trivial>>,
                         TypeHelper<NonTrivial>,
                         TypeHelper<NoMove>>
    GenericTestCases;

TYPED_TEST_CASE(SBOVector_, GenericTestCases);

template <typename Range1, typename Range2>
void EXPECT_RANGE_EQ(const Range1& A, const Range2& B) {
  auto begin1 = A.begin();
  auto begin2 = B.begin();
  auto end1 = A.end();
  auto end2 = B.end();
  ASSERT_EQ(std::distance(begin1, end1), std::distance(begin2, end2))
      << "Ranges must be of equal size!\n";
  auto iter1 = begin1;
  auto iter2 = begin2;
  for (; iter1 != end1; ++iter1, ++iter2) {
    EXPECT_EQ(*iter1, *iter2) << "Element mismatch at position "
                              << std::distance(begin1, iter1) << "\n";
  }
}

template <size_t... Values>
constexpr std::vector<int> vector_from_sequence() {
  return std::vector<int>{Values...};
}

template <int... Values>
constexpr std::vector<int> vector_from_sequence(
    std::integer_sequence<size_t, Values...>) {
  return vector_from_sequence<Values...>();
}

template <size_t V>
constexpr std::vector<int> make_vector_sequence() {
  return vector_from_sequence(std::make_index_sequence<V>());
}

#endif  // UNITTEST_COMMON_HPP
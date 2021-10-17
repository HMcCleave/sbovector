#include <gtest/gtest.h>

#include "sbovector.hpp"

#include <memory>

constexpr size_t SMALL_SIZE = 5;
constexpr size_t LARGE_SIZE = 100;
constexpr size_t SBO_SIZE = 16;

static_assert(SMALL_SIZE < SBO_SIZE);
static_assert(SBO_SIZE < LARGE_SIZE);

template<typename T>
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

template<typename T>
struct CustomAllocator {
  int dummy_;
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;

  pointer allocate(size_t n, const void*) { return allocate(n); }
  pointer allocate(size_t n) {
    return new T[n];
  }
  void deallocate(pointer p, size_t) {
    delete[] p;
  }
};


using Trivial = int;
class NonTrivial {
 private:
  std::unique_ptr<int> val_;

 public:
  NonTrivial() { 
    val_ = std::make_unique<int>(42);
  }

  ~NonTrivial() {}

  NonTrivial(const NonTrivial&) {}

  NonTrivial(NonTrivial&&) noexcept {}

  NonTrivial& operator=(const NonTrivial& other) {
    return *this;
  }

  NonTrivial& operator=(NonTrivial&& other) noexcept {
    return *this;
  }
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

template<typename Data, typename Alloc = std::allocator<Data>>
struct TypeHelper {
  typedef typename Data DataType;
  typedef typename Alloc AllocatorType;
};

template<typename T>
struct SBOVector_ : public ::testing::Test {
  using DataType = typename T::DataType;
  using AllocatorType = typename T::AllocatorType;
  using ContainerType = SBOVector<DataType, SBO_SIZE, AllocatorType>;
};

typedef ::testing::Types<
  TypeHelper<Trivial>,
  TypeHelper<Trivial, CustomAllocator<Trivial>>,
  TypeHelper<NonTrivial>,
  TypeHelper<NoMove>
> GenericTestCases;

TYPED_TEST_CASE(SBOVector_, GenericTestCases);

TYPED_TEST(SBOVector_, MustDefaultConstruct) {
  ContainerType container{};
  EXPECT_EQ(container.size(), 0);
  EXPECT_TRUE(container.empty());
}

TYPED_TEST(SBOVector_, MustConstructSmallNumberOfCopies) {
  ContainerType container(SMALL_SIZE);
  EXPECT_EQ(container.size(), SMALL_SIZE);
  EXPECT_EQ(container.capacity(), SBO_SIZE);
  EXPECT_FALSE(container.empty());
}

TYPED_TEST(SBOVector_, MustConstructLargeNumberOfCopies) {
  ContainerType container(LARGE_SIZE);
  EXPECT_EQ(container.size(), LARGE_SIZE);
  EXPECT_GE(container.capacity(), LARGE_SIZE);
  EXPECT_FALSE(container.empty());
}

TEST(SBOVectorOfInts, MustNotAllocateOnSmallConstruction) {
  CountingAllocator<int>::Totals totals{};
  CountingAllocator<int> alloc(&totals);
  SBOVector<int, SBO_SIZE, CountingAllocator<int>> container(
      SMALL_SIZE, alloc);
  EXPECT_EQ(totals.allocs_, 0);
}

TEST(SBOVector, MustDeallocateAllAllocatinosOnLargeConstruction) {
  CountingAllocator<int>::Totals totals{};
  CountingAllocator<int> alloc(&totals);
  {
    SBOVector<int, SBO_SIZE, CountingAllocator<int>> container(LARGE_SIZE, alloc);
    EXPECT_EQ(container.size(), LARGE_SIZE);
    EXPECT_GE(container.capacity(), LARGE_SIZE);
    EXPECT_FALSE(container.empty());
    EXPECT_NE(totals.allocs_, 0);
  }
  EXPECT_EQ(totals.allocs_, totals.frees_);
}

TYPED_TEST(SBOVector_, MustIteratorConstructSmallCollection) {
  std::vector<DataType> vec(SMALL_SIZE, DataType());
  ContainerType container(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), SMALL_SIZE);
  EXPECT_EQ(container.capacity(), SBO_SIZE);
  EXPECT_FALSE(container.empty());
}

TYPED_TEST(SBOVector_, MustIteratorConstructLargeCollection) {
  std::vector<DataType> vec(LARGE_SIZE, DataType());
  ContainerType container(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), LARGE_SIZE);
  EXPECT_GE(container.capacity(), LARGE_SIZE);
  EXPECT_FALSE(container.empty());
}

TYPED_TEST(SBOVector_, MustConstructFromInitializerList) {
  std::initializer_list<DataType> list{
      DataType(), DataType(), DataType(), DataType(),
      DataType(), DataType(), DataType(), DataType(),
  };
  ContainerType container(list);
  EXPECT_EQ(container.size(), list.size());
  EXPECT_FALSE(container.empty());
}

struct CopyMoveCounter {
  static int s_move_count_;
  static int s_copy_count_;
  CopyMoveCounter(CopyMoveCounter&&) noexcept { ++s_move_count_; }
  CopyMoveCounter(const CopyMoveCounter&) { ++s_copy_count_; }
  CopyMoveCounter() {}
  ~CopyMoveCounter() {}
  CopyMoveCounter& operator=(CopyMoveCounter&&) noexcept {
    ++s_move_count_;
    return *this;
  }
  CopyMoveCounter& operator=(const CopyMoveCounter&) { 
    ++s_copy_count_;
    return *this;
  }
};


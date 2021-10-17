#include <gtest/gtest.h>

#include "sbovector.hpp"

constexpr int SMALL_SIZE = 5;
constexpr int LARGE_SIZE = 100;
constexpr int SBO_SIZE = 16;

template<typename T>
struct CountingAllocator {
  int* counter_;
  CountingAllocator(int* counter) : counter_(counter) {}

  T* allocate(size_t n, const void*) { return allocate(n); }
  T* allocate(size_t n) { 
    ++(*counter_);
    return new T[n];
  }
  void deallocate(T* p, size_t n) { 
    --(*counter_);
    delete[] p;
  }
};

using ContainerType = SBOVector<int, SBO_SIZE>; // SBOVector<int, SBO_SIZE>;

TEST(SBOVectorOfInts, MustDefaultConstruct) {
  ContainerType container{};
  EXPECT_EQ(container.size(), 0);
  EXPECT_TRUE(container.empty());
}

TEST(SBOVectorOfInts, MustConstructWithCustomAllocator) {
  int alloc_count = 0;
  CountingAllocator<int> alloc(&alloc_count);
  SBOVector<int, SBO_SIZE, CountingAllocator<int>> container(alloc);
  EXPECT_EQ(alloc_count, 0);
  EXPECT_EQ(container.size(), 0);
  EXPECT_TRUE(container.empty());
}

static_assert(SMALL_SIZE < SBO_SIZE);
static_assert(SBO_SIZE < LARGE_SIZE);

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

struct Unmovable {
  Unmovable() {}
  Unmovable(Unmovable&&) = delete;
  Unmovable(const Unmovable&) {}
  ~Unmovable() {}
  Unmovable& operator=(Unmovable&&) = delete;
  Unmovable& operator=(const Unmovable&) {}
};

template<int InitialSize>
class ContainerTestBase : public testing::Test {
  protected:
  void setup() { 
    container_ = ContainerType{};
    for (auto i = 0; i < InitialSize; ++i) {
      container_.push_back(i);
    }
  }
  void teardown() { container_.clear(); }
  const int initial_size_ = InitialSize;
  ContainerType container_;
};

using AnEmptyContainer = ContainerTestBase<0>;
using ASmallContainer = ContainerTestBase<SMALL_SIZE>;
using ALargeContainer = ContainerTestBase<LARGE_SIZE>;

TEST_F(AnEmptyContainer, MustBeInitiallyEmpty) {
  EXPECT_EQ(this->container_.size(), 0);
  EXPECT_TRUE(this->container_.empty());
}


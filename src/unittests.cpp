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

  NonTrivial(const NonTrivial&) : NonTrivial() {}

  NonTrivial(NonTrivial&&) noexcept : NonTrivial() {}

  NonTrivial& operator=(const NonTrivial&) {
    return *this;
  }

  NonTrivial& operator=(NonTrivial&&) noexcept {
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

TYPED_TEST(SBOVector_, MustIterateCorrectNumberOfElementsWithInternalBuffer) {
  ContainerType container(SMALL_SIZE, DataType());
  auto beg = container.begin();
  auto end = container.end();
  auto iter_size = std::distance(beg, end);
  EXPECT_EQ(SMALL_SIZE, iter_size);
  auto cbeg = container.cbegin();
  auto cend = container.cend();
  iter_size = std::distance(cbeg, cend);
  EXPECT_EQ(SMALL_SIZE, iter_size);
}

TYPED_TEST(SBOVector_, MustIterateCorrectNumberOfElementsWithExternalBuffer) {
  ContainerType container(LARGE_SIZE, DataType());
  auto beg = container.begin();
  auto end = container.end();
  auto iter_size = std::distance(beg, end);
  EXPECT_EQ(LARGE_SIZE, iter_size);
  auto cbeg = container.cbegin();
  auto cend = container.cend();
  iter_size = std::distance(cbeg, cend);
  EXPECT_EQ(LARGE_SIZE, iter_size);
}

TEST(SBOVectorOfInts, MustIterateOverCorrectValuesWithInternalBuffer) {
  std::vector<int> vec{1, 5, 3, 2, 4};
  SBOVector<int, SBO_SIZE> container{vec.begin(), vec.end()};
  auto ref_iter = vec.begin();
  for (auto iter = container.begin(); iter != container.end();
       ++iter, ++ref_iter) {
    EXPECT_EQ(*iter, *ref_iter);
  }
}

TEST(SBOVectorOfInts, MustIterateOverCorrectValuesWithExternalBuffer) {
  std::vector<int> vec{1, 5, 3, 2, 4, 6, 45, 32, 11, -2, 7, 15, 3, 28, 6, 4, 5, 2, 1, 2, 56};
  SBOVector<int, SBO_SIZE> container{vec.begin(), vec.end()};
  auto ref_iter = vec.begin();
  for (auto iter = container.begin(); iter != container.end();
       ++iter, ++ref_iter) {
    EXPECT_EQ(*iter, *ref_iter);
  }
}

TYPED_TEST(SBOVector_, PopBackMustReduceSizeWithInternalBuffer) {
  ContainerType container(SMALL_SIZE);
  container.pop_back();
  EXPECT_EQ(container.size(), SMALL_SIZE - 1);
}

TYPED_TEST(SBOVector_, PopBackMustReduceSizeWithExternalBuffer) {
  ContainerType container(LARGE_SIZE);
  container.pop_back();
  EXPECT_EQ(container.size(), LARGE_SIZE - 1);
}

TYPED_TEST(SBOVector_,
  PopBackMustReduceSizeFromExternalBufferToInternalBuffer) {
  ContainerType container(SBO_SIZE + 1);
  container.pop_back();
  EXPECT_EQ(container.size(), SBO_SIZE);
}

TYPED_TEST(SBOVector_, PushBackCopyMustHandleInternalBuffer) {
  ContainerType container(SMALL_SIZE);
  const DataType c_data{};
  container.push_back(c_data);
  EXPECT_EQ(container.size(), SMALL_SIZE + 1);
}

TYPED_TEST(SBOVector_, PushBackCopyMustHandleExternalBuffer) {
  ContainerType container(LARGE_SIZE);
  const DataType c_data{};
  container.push_back(c_data);
  EXPECT_EQ(container.size(), LARGE_SIZE + 1);
}

TYPED_TEST(SBOVector_, PushBackCopyMustGrow) {
  ContainerType container(SBO_SIZE);
  const DataType c_data{};
  container.push_back(c_data);
  EXPECT_EQ(container.size(), SBO_SIZE + 1);
}

TYPED_TEST(SBOVector_, PushBackMoveMustHandleInternalBuffer) {
  ContainerType container(SMALL_SIZE);
  DataType m_data{};
  container.push_back(std::move(m_data));
  EXPECT_EQ(container.size(), SMALL_SIZE + 1);
}

TYPED_TEST(SBOVector_, PushBackMoveMustHandleExternalBuffer) {
  ContainerType container(LARGE_SIZE);
  DataType m_data{};
  container.push_back(std::move(m_data));
  EXPECT_EQ(container.size(), LARGE_SIZE + 1);
}

TYPED_TEST(SBOVector_, PushBackMoveMustGrow) {
  ContainerType container(SBO_SIZE);
  DataType m_data{};
  container.push_back(std::move(m_data));
  EXPECT_EQ(container.size(), SBO_SIZE + 1);
}

TYPED_TEST(SBOVector_, EmplaceMustHandleInternalBuffer) {
  ContainerType container(SMALL_SIZE);
  container.emplace_back();
  EXPECT_EQ(container.size(), SMALL_SIZE + 1);
}

TYPED_TEST(SBOVector_, EmplaceMustHandleExternalBuffer) {
  ContainerType container(LARGE_SIZE);
  container.emplace_back();
  EXPECT_EQ(container.size(), LARGE_SIZE + 1);
}

TYPED_TEST(SBOVector_, EmplaceMoveMustGrow) {
  ContainerType container(SBO_SIZE);
  container.emplace_back();
  EXPECT_EQ(container.size(), SBO_SIZE + 1);
}

TYPED_TEST(SBOVector_, MustReserveIfExternal) {
  ContainerType container(LARGE_SIZE);
  container.reserve_if_external(LARGE_SIZE * 2);
  EXPECT_EQ(container.capacity(), LARGE_SIZE * 2);
}

TYPED_TEST(SBOVector_, MustShrinkToFitIfExternal) {
  ContainerType container(LARGE_SIZE);
  container.reserve_if_external(LARGE_SIZE * 2);
  container.shrink_to_fit_if_external();
  EXPECT_EQ(container.size(), container.capacity());
}

TYPED_TEST(SBOVector_, MustSwapWithEmptyContainer) {
  ContainerType first;
  ContainerType second(SMALL_SIZE);
  ContainerType third(LARGE_SIZE);
  first.swap(second);
  second.swap(third);
  EXPECT_EQ(third.size(), 0);
  EXPECT_EQ(second.size(), LARGE_SIZE);
  EXPECT_EQ(first.size(), SMALL_SIZE);
}

TYPED_TEST(SBOVector_, MustSwapInternalBuffers) {
  ContainerType first(SMALL_SIZE);
  ContainerType second(SMALL_SIZE + 1);
  first.swap(second);
  EXPECT_EQ(first.size(), SMALL_SIZE + 1);
  EXPECT_EQ(second.size(), SMALL_SIZE);
}

TYPED_TEST(SBOVector_, MustSwapExternalBuffers) {
  ContainerType first(LARGE_SIZE);
  ContainerType second(LARGE_SIZE + 1);
  second.swap(first);
  EXPECT_EQ(first.size(), LARGE_SIZE + 1);
  EXPECT_EQ(second.size(), LARGE_SIZE);
}

TYPED_TEST(SBOVector_, MustSwapInternalAndExternalBuffer) {
  ContainerType first(SMALL_SIZE);
  ContainerType second(LARGE_SIZE);
  first.swap(second);
  EXPECT_EQ(first.size(), LARGE_SIZE);
  EXPECT_EQ(second.size(), SMALL_SIZE);
}

TYPED_TEST(SBOVector_, MustSwapInternalBuffersOfDifferentSize) {
  ContainerType first(SMALL_SIZE);
  SBOVector<DataType, SBO_SIZE + 10, AllocatorType> second(SMALL_SIZE + 1);
  static_assert(SBO_SIZE >= SMALL_SIZE + 1);
  first.swap(second);
  EXPECT_EQ(first.size(), SMALL_SIZE + 1);
  EXPECT_EQ(second.size(), SMALL_SIZE);
}

TYPED_TEST(SBOVector_, MustSwapToSmallerContainer) { /*
  ContainerType first(SMALL_SIZE);
  SBOVector<DataType, SMALL_SIZE - 1, AllocatorType> second(SMALL_SIZE - 2);
  static_assert(SMALL_SIZE > 2);
  first.swap(second);
  EXPECT_EQ(first.size(), SMALL_SIZE - 2);
  EXPECT_EQ(second.size(), SMALL_SIZE);*/
}


struct OperationCounter {
  struct OperationTotals {
    int default_constructor_{0};
    int copy_constructor_{0};
    int move_constructor_{0};
    int copy_assignment_{0};
    int move_assignment_{0};
    int moved_destructor_{0};
    int unmoved_destructor_{0};
    void reset() {
      memset(this, 0, sizeof(this));
    }
    int moves() const { return move_constructor_ + move_assignment_; }
    int copies() const { return copy_constructor_ + copy_assignment_; }
    int constructs() const {
      return default_constructor_ + copy_constructor_ + move_constructor_;
    }
    int destructs() const { return moved_destructor_ + unmoved_destructor_; }
  };
  bool moved_ { false };
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

TEST(SBOVectorInternalBuffer, PopBackMustTriggerSingleDestructor) {
  SBOVector<OperationCounter, SBO_SIZE> container(SMALL_SIZE);
  auto count_before = OperationCounter::TOTALS.destructs();
  container.pop_back();
  auto count_after = OperationCounter::TOTALS.destructs();
  EXPECT_EQ(count_before + 1, count_after);
}

TEST(SBOVectorPushBack, MustDeallocateOldData) {
  CountingAllocator<OperationCounter>::Totals alloc_totals{};
  OperationCounter::TOTALS.reset();
  {
    SBOVector<OperationCounter, SBO_SIZE, CountingAllocator<OperationCounter>>
        container(LARGE_SIZE, {&alloc_totals});
    while (container.size() < container.capacity())
      container.push_back({});
    container.push_back({});
  }
  auto& Totals = OperationCounter::TOTALS;
  EXPECT_EQ(Totals.destructs(), Totals.constructs());
  EXPECT_EQ(alloc_totals.allocs_, alloc_totals.frees_);
}

TEST(SBOVectorReserve, MustMatchAlloc_Free) {
  CountingAllocator<OperationCounter>::Totals alloc_totals{};
  {
    SBOVector<OperationCounter, SBO_SIZE, CountingAllocator<OperationCounter>>
        container(LARGE_SIZE, {&alloc_totals});
    container.reserve_if_external(LARGE_SIZE * 2);
  }
  EXPECT_EQ(alloc_totals.allocs_, alloc_totals.frees_);
}

TEST(SBOVectorShrinkToFit, MustMatchAlloc_Free) {
  CountingAllocator<OperationCounter>::Totals alloc_totals{};
  {
    SBOVector<OperationCounter, SBO_SIZE, CountingAllocator<OperationCounter>>
        container(LARGE_SIZE, {&alloc_totals});
    container.reserve_if_external(LARGE_SIZE * 2);
    container.shrink_to_fit_if_external();
  }
  EXPECT_EQ(alloc_totals.allocs_, alloc_totals.frees_);
}
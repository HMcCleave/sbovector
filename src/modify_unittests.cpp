#include "unittest_common.hpp"

// Unittests for insert, erase, push/pop _back, emplace, clear, resize methods

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

TYPED_TEST(SBOVector_, MustEmplaceValue) {
  const DataType t{};
  {  // Small Container
    ContainerType small(SMALL_SIZE);
    auto out = small.emplace(small.begin());
    EXPECT_EQ(out, small.begin());
    EXPECT_EQ(small.size(), SMALL_SIZE + 1);
  }
  {  // Transitioning Container
    ContainerType full(SBO_SIZE);
    full.emplace(full.begin());
    EXPECT_EQ(full.size(), SBO_SIZE + 1);
  }
  {  // Large Container
    ContainerType large(LARGE_SIZE);
    large.emplace(large.begin());
    EXPECT_EQ(large.size(), LARGE_SIZE + 1);
  }
}

TYPED_TEST(SBOVector_, MustInsertSingleValue) {
  const DataType t{};
  {  // Small Container
    ContainerType small(SMALL_SIZE);
    auto out = small.insert(small.begin(), DataType());
    EXPECT_EQ(out, small.begin());
    EXPECT_EQ(small.size(), SMALL_SIZE + 1);
  }
  {  // Transitioning Container
    ContainerType full(SBO_SIZE);
    full.insert(full.begin(), DataType());
    EXPECT_EQ(full.size(), SBO_SIZE + 1);
  }
  {  // Large Container
    ContainerType large(LARGE_SIZE);
    large.insert(large.begin(), DataType());
    EXPECT_EQ(large.size(), LARGE_SIZE + 1);
  }
}
TEST(SBOVector_NonTrivial, MustMoveInsertSingleValue) {
  using ContainerType = SBOVector<NonTrivial, SBO_SIZE>;
  using DataType = NonTrivial;
  {  // Small Container
    ContainerType small(SMALL_SIZE);
    DataType t;
    small.insert(small.begin(), std::move(t));
    EXPECT_EQ(small.size(), SMALL_SIZE + 1);
  }
  {  // Transitioning Container
    ContainerType full(SBO_SIZE);
    DataType t;
    full.insert(full.begin(), std::move(t));
    EXPECT_EQ(full.size(), SBO_SIZE + 1);
  }
  {  // Large Container
    ContainerType large(LARGE_SIZE);
    DataType t;
    large.insert(large.begin(), std::move(t));
    EXPECT_EQ(large.size(), LARGE_SIZE + 1);
  }
}
TEST(SBOVector_NonTrivial, MustInsertMultipleValues) {
  using ContainerType = SBOVector<NonTrivial, SBO_SIZE>;
  using DataType = NonTrivial;
  {  // Small Container
    ContainerType small(SMALL_SIZE);
    DataType t;
    small.insert(small.begin(), 2u, t);
    EXPECT_EQ(small.size(), SMALL_SIZE + 2);
  }
  {  // Transitioning Container
    ContainerType full(SBO_SIZE);
    DataType t;
    full.insert(full.begin(), 5u, t);
    EXPECT_EQ(full.size(), SBO_SIZE + 5);
  }
  {  // Large Container
    ContainerType large(LARGE_SIZE);
    DataType t;
    large.insert(large.begin(), 15u, t);
    EXPECT_EQ(large.size(), LARGE_SIZE + 15);
  }
}

TYPED_TEST(SBOVector_, MustEraseSingleValue) {
  {
    ContainerType small(SMALL_SIZE);
    small.erase(small.begin() + 1);
    EXPECT_EQ(small.size(), SMALL_SIZE - 1);
  }
  {
    ContainerType shrinking(SBO_SIZE + 1);
    shrinking.erase(shrinking.begin());
    EXPECT_EQ(shrinking.size(), SBO_SIZE);
  }
  {
    ContainerType large(LARGE_SIZE);
    large.erase(large.end() - 1);
    EXPECT_EQ(large.size(), LARGE_SIZE - 1);
  }
}

TYPED_TEST(SBOVector_, MustEraseRange) {
  {
    ContainerType small(SMALL_SIZE);
    small.erase(small.begin() + 1, small.end());
    EXPECT_EQ(small.size(), 1);
  }
  {
    ContainerType shrinking(SBO_SIZE + 1);
    shrinking.erase(shrinking.begin() + 1, shrinking.end());
    EXPECT_EQ(shrinking.size(), 1);
  }
  {
    ContainerType large(LARGE_SIZE);
    large.erase(large.begin(), large.end() - 1);
    EXPECT_EQ(large.size(), 1);
  }
}

TYPED_TEST(SBOVector_, MustResize) {
  ContainerType c(SMALL_SIZE);
  c.resize(SBO_SIZE);
  EXPECT_EQ(c.size(), SBO_SIZE);
  c.resize(LARGE_SIZE, DataType());
  EXPECT_EQ(c.size(), LARGE_SIZE);
  c.resize(SMALL_SIZE);
  EXPECT_EQ(c.size(), SMALL_SIZE);
}

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
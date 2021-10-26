#include "unittest_common.hpp"

// Unittests for operator=, assign methods
TYPED_TEST(SBOVector_, MustCopyAssignSmall) {
  const ContainerType original(SMALL_SIZE);
  ContainerType copy{};
  copy = original;
  EXPECT_EQ(copy.size(), original.size());
}

TEST_F(OperationTrackingSBOVector, MustCopyAssignSmall) {
  {
    const ContainerType original(SMALL_SIZE, create_allocator());
    ContainerType copy(create_allocator());
    copy = original;
    EXPECT_EQ(copy.size(), original.size());
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
}

TEST(SBOVectorOfInts, MustCopyAssignSmall) {
  auto vec = make_vector_sequence<SMALL_SIZE>();
  const SBOVector<int, SBO_SIZE> original{vec.begin(), vec.end()};
  SBOVector<int, SBO_SIZE> copy{};
  copy = original;
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustCopyAssignLarge) {
  const ContainerType original(LARGE_SIZE);
  ContainerType copy{};
  copy = original;
  EXPECT_EQ(copy.size(), original.size());
}

TEST_F(OperationTrackingSBOVector, MustCopyAssignLarge) {
  {
    const ContainerType original(LARGE_SIZE, create_allocator());
    ContainerType copy(create_allocator());
    copy = original;
    EXPECT_EQ(copy.size(), original.size());
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
}

TEST(SBOVectorOfInts, MustCopyAssignLarge) {
  auto vec = make_vector_sequence<LARGE_SIZE>();
  const SBOVector<int, SBO_SIZE> original(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> copy{};
  copy = original;
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustCopyAssignAsymetric) {
  const ContainerType original(LARGE_SIZE);
  SBOVector<DataType, SMALL_SIZE, AllocatorType> copy{};
  copy = original;
  EXPECT_EQ(original.size(), copy.size());
}

TEST_F(OperationTrackingSBOVector, MustCopyAssignAsymmetric) {
  {
    const ContainerType original(LARGE_SIZE, create_allocator());
    SBOVector<DataType, SMALL_SIZE, AllocatorType> copy{create_allocator()};
    copy = original;
    EXPECT_EQ(original.size(), copy.size());
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
}

TEST(SBOVectorOfInts, MustCopyAssignAsymmetric) {
  auto vec = make_vector_sequence<LARGE_SIZE>();
  const SBOVector<int, SBO_SIZE> original{vec.begin(), vec.end()};
  SBOVector<int, SMALL_SIZE, CustomAllocator<int>> copy{};
  copy = original;
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustMoveAssignSmall) {
  ContainerType original(SMALL_SIZE);
  ContainerType copy{};
  copy = std::move(original);
  EXPECT_EQ(copy.size(), SMALL_SIZE);
}

TEST_F(OperationTrackingSBOVector, MustMoveAssignSmall) {
  {
    ContainerType original(SMALL_SIZE, create_allocator());
    ContainerType copy(create_allocator());
    copy = std::move(original);
    EXPECT_EQ(copy.size(), SMALL_SIZE);
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
}

TEST(SBOVectorOfInts, MustMoveAssignSmall) {
  auto vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> original(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> copy{};
  copy = std::move(original);
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustMoveAssignLarge) {
  ContainerType original(LARGE_SIZE);
  ContainerType copy{};
  copy = std::move(original);
  EXPECT_EQ(copy.size(), LARGE_SIZE);
}

TEST_F(OperationTrackingSBOVector, MustMoveAssignLarge) {
  {
    ContainerType original(LARGE_SIZE, create_allocator());
    ContainerType copy(create_allocator());
    copy = std::move(original);
    EXPECT_EQ(copy.size(), LARGE_SIZE);
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
}

TEST(SBOVectorOfInts, MustMoveAssignLarge) {
  auto vec = make_vector_sequence<LARGE_SIZE>();
  SBOVector<int, SBO_SIZE> original{vec.begin(), vec.end()};
  SBOVector<int, SBO_SIZE> copy{};
  copy = std::move(original);
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustMoveAssignAsymetric) {
  ContainerType original(LARGE_SIZE);
  SBOVector<DataType, SMALL_SIZE, AllocatorType> copy{};
  copy = std::move(original);
  EXPECT_EQ(LARGE_SIZE, copy.size());
}

TEST_F(OperationTrackingSBOVector, MustMoveAssignAsymmetric) {
  {
    ContainerType original(LARGE_SIZE, create_allocator());
    SBOVector<DataType, SMALL_SIZE, AllocatorType> copy(create_allocator());
    copy = std::move(original);
    EXPECT_EQ(LARGE_SIZE, copy.size());
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
}

TEST(SBOVectorOfInts, MustMoveAssignAsymmetric) {
  auto vec = make_vector_sequence<LARGE_SIZE>();
  SBOVector<int, SBO_SIZE> original(vec.begin(), vec.end());
  SBOVector<int, SMALL_SIZE, CustomAllocator<int>> copy{};
  copy = std::move(original);
  EXPECT_RANGE_EQ(copy, vec);
}
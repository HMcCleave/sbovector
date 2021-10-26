#include "unittest_common.hpp"

// Unittests for operator=, assign methods
TYPED_TEST(SBOVector_, MustCopyAssignSmall) {
  const ContainerType original{SMALL_SIZE};
  ContainerType copy{};
  copy = original;
  EXPECT_EQ(copy.size(), original.size());
}

TEST_F(OperationTrackingSBOVector, MustCopyAssignSmall) {
  {
    const ContainerType original{SMALL_SIZE, create_allocator()};
    ContainerType copy{create_allocator()};
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
#include "unittest_common.hpp"

// Unittests for capacity related methods (capacity, reserve, shrink_to_fit)

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
#include "unittest_common.hpp"

// Unittests for capacity related methods (capacity, reserve, shrink_to_fit)

TYPED_TEST(SBOVector_, MustReserveIfExternal) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  ContainerType container(LARGE_SIZE);
  container.reserve_if_external(LARGE_SIZE * 2);
  EXPECT_EQ(container.capacity(), LARGE_SIZE * 2);
}

TEST_F(DataTypeOperationTrackingSBOVector, MustReserveIfExternal) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  ContainerType container(LARGE_SIZE, create_allocator());
  container.reserve_if_external(LARGE_SIZE * 2);
  UseElements(container);
}

TYPED_TEST(SBOVector_, MustShrinkToFitIfExternal) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  ContainerType container(LARGE_SIZE);
  container.reserve_if_external(LARGE_SIZE * 2);
  container.shrink_to_fit_if_external();
  EXPECT_EQ(container.size(), container.capacity());
}
TEST_F(DataTypeOperationTrackingSBOVector, MustShrinkToFitIfExternal) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  ContainerType container(LARGE_SIZE, create_allocator());
  container.reserve_if_external(LARGE_SIZE * 2);
  container.shrink_to_fit_if_external();
  UseElements(container);
}
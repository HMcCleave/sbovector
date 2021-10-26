#include "unittest_common.hpp"

// Unittests for swap methods

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

TYPED_TEST(SBOVector_, MustSwapToSmallerContainer) { 
  ContainerType first(SMALL_SIZE);
  SBOVector<DataType, SMALL_SIZE - 1, AllocatorType> second(SMALL_SIZE - 2);
  static_assert(SMALL_SIZE > 2);
  first.swap(second);
  EXPECT_EQ(first.size(), SMALL_SIZE - 2);
  EXPECT_EQ(second.size(), SMALL_SIZE);
}

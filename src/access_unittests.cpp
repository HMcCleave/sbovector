#include "unittest_common.hpp"

// Unittests for iteration (begin/end and variations there-of), at, data, size, operator[] methods

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

TEST(ValueVerifiedSBOVector, MustIterateOverCorrectValuesWithInternalBuffer) {
  std::vector<int> vec{1, 5, 3, 2, 4};
  SBOVector<int, SBO_SIZE> container{vec.begin(), vec.end()};
  EXPECT_RANGE_EQ(container, vec);
}

TEST(ValueVerifiedSBOVector, MustIterateOverCorrectValuesWithExternalBuffer) {
  std::vector<int> vec{1,  5, 3,  2, 4, 6, 45, 32, 11, -2, 7,
                       15, 3, 28, 6, 4, 5, 2,  1,  2,  56};
  SBOVector<int, SBO_SIZE> container{vec.begin(), vec.end()};
  EXPECT_RANGE_EQ(vec, container);
}
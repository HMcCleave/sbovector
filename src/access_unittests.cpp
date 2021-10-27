#include "unittest_common.hpp"

// Unittests for iteration (begin/end and variations there-of), at, data, size, operator[] methods

TYPED_TEST(SBOVector_, MustIterateViaMutableBeginEnd) {
  size_t count{0};
  auto null_op = [&](DataType&) { ++count; };
  ContainerType container(SMALL_SIZE);
  for (auto iter = container.begin(); iter != container.end(); ++iter)
    null_op(*iter);
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  container.resize(LARGE_SIZE);
  for (auto iter = container.begin(); iter != container.end(); ++iter) {
    null_op(*iter);
  }
  EXPECT_EQ(count, LARGE_SIZE);
}

TEST(ValueVerifiedSBOVector, MustIterateViaMutableBeginEnd) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  int count = 0;
  for (int* iter = sbo.begin(); iter != sbo.end(); ++iter) {
    EXPECT_EQ(*iter, vec[count++]);
  }
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (int* iter = sbo.begin(); iter != sbo.end(); ++iter) {
    EXPECT_EQ(*iter, vec[count++]);
  }
  EXPECT_EQ(count, LARGE_SIZE);
}

TYPED_TEST(SBOVector_, MustIterateViaConstBeginEnd) {
  size_t count{0};
  auto null_op = [&](const DataType&) { ++count; };
  const ContainerType small(SMALL_SIZE), large(LARGE_SIZE);
  for (auto iter = small.begin(); iter != small.end(); ++iter)
    null_op(*iter);
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  for (auto iter = large.begin(); iter != large.end(); ++iter) 
    null_op(*iter);
  EXPECT_EQ(count, LARGE_SIZE);
}

TEST(ValueVerifiedSBOVector, MustIterateViaConstBeginEnd) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  const SBOVector<int, SBO_SIZE> sbo_small(vec.begin(), vec.end());
  int count = 0;
  for (const int* iter = sbo_small.begin(); iter != sbo_small.end(); ++iter, ++count) {
    EXPECT_EQ(*iter, vec[count]);
  }
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  vec = make_vector_sequence<LARGE_SIZE>();
  const SBOVector<int, SBO_SIZE> sbo_large(vec.begin(), vec.end());
  for (const int* iter = sbo_large.begin(); iter != sbo_large.end(); ++iter, ++count) {
    EXPECT_EQ(*iter, vec[count]);
  }
  EXPECT_EQ(count, LARGE_SIZE);
}

TYPED_TEST(SBOVector_, MustIterateViaCBeginCEnd) {
  size_t count{0};
  auto null_op = [&](const DataType&) { ++count; };
  const ContainerType small(SMALL_SIZE), large(LARGE_SIZE);
  for (auto iter = small.cbegin(); iter != small.cend(); ++iter)
    null_op(*iter);
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  for (auto iter = large.cbegin(); iter != large.cend(); ++iter)
    null_op(*iter);
  EXPECT_EQ(count, LARGE_SIZE);
}

TEST(ValueVerifiedSBOVector, MustIterateViaCBeginCEnd) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  const SBOVector<int, SBO_SIZE> sbo_small(vec.begin(), vec.end());
  int count = 0;
  for (const int* iter = sbo_small.cbegin(); iter != sbo_small.cend();
       ++iter, ++count) {
    EXPECT_EQ(*iter, vec[count]);
  }
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  vec = make_vector_sequence<LARGE_SIZE>();
  const SBOVector<int, SBO_SIZE> sbo_large(vec.begin(), vec.end());
  for (const int* iter = sbo_large.cbegin(); iter != sbo_large.cend();
       ++iter, ++count) {
    EXPECT_EQ(*iter, vec[count]);
  }
  EXPECT_EQ(count, LARGE_SIZE);
}

TEST(ValueVerifiedSBOVector, MustAccessViaAt) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec.at(i), sbo.at(i));
  }
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec.at(i), sbo.at(i));
  }
}

TEST(ValueVerifiedSBOVector, MustAccessViaIndexOperator) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec[i], sbo[i]);
  }
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec[i], sbo[i]);
  }
}

TEST(ValueVerifiedSBOVector, MustAccessViaData) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec.data()[i], sbo.data()[i]);
  }
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec.data()[i], sbo.data()[i]);
  }
}

TYPED_TEST(SBOVector_, MustReportSize) {
  ContainerType empty{}, small(SMALL_SIZE), large(LARGE_SIZE);
  EXPECT_EQ(empty.size(), 0);
  EXPECT_EQ(small.size(), SMALL_SIZE);
  EXPECT_EQ(large.size(), LARGE_SIZE);
}
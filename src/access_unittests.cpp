#include "unittest_common.hpp"

// Unittests for iteration (begin/end and variations there-of), at, front/back data, size, operator[] methods

TYPED_TEST(SBOVector_, MustIterateViaMutableBeginEnd) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
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
  size_t count = 0;
  for (int* iter = sbo.begin(); iter != sbo.end(); ++iter, ++count) {
    EXPECT_EQ(*iter, vec[count]);
  }
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (int* iter = sbo.begin(); iter != sbo.end(); ++iter, ++count) {
    EXPECT_EQ(*iter, vec[count]);
  }
  EXPECT_EQ(count, LARGE_SIZE);
}

TYPED_TEST(SBOVector_, MustIterateViaConstBeginEnd) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
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
  size_t count = 0;
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
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
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
  size_t count = 0;
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

TYPED_TEST(SBOVector_, MustIterateViaMutableRBeginREnd) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  size_t count{0};
  auto null_op = [&](DataType&) { ++count; };
  ContainerType container(SMALL_SIZE);
  for (auto iter = container.rbegin(); iter != container.rend(); ++iter)
    null_op(*iter);
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  container.resize(LARGE_SIZE);
  for (auto iter = container.rbegin(); iter != container.rend(); ++iter) {
    null_op(*iter);
  }
  EXPECT_EQ(count, LARGE_SIZE);
}

TEST(ValueVerifiedSBOVector, MustIterateViaMutableRBeginREnd) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  int count = 0;
  for (auto iter = sbo.rbegin(); iter != sbo.rend(); ++iter, ++count) {
    EXPECT_EQ(*iter, *(vec.rbegin() + count));
  }
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (auto iter = sbo.rbegin(); iter != sbo.rend(); ++iter, ++count) {
    EXPECT_EQ(*iter, *(vec.rbegin() + count));
  }
  EXPECT_EQ(count, LARGE_SIZE);
}

TYPED_TEST(SBOVector_, MustIterateViaConstRBeginREnd) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  size_t count{0};
  auto null_op = [&](const DataType&) { ++count; };
  const ContainerType small(SMALL_SIZE), large(LARGE_SIZE);
  for (auto iter = small.rbegin(); iter != small.rend(); ++iter)
    null_op(*iter);
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  for (auto iter = large.rbegin(); iter != large.rend(); ++iter)
    null_op(*iter);
  EXPECT_EQ(count, LARGE_SIZE);
}

TEST(ValueVerifiedSBOVector, MustIterateViaConstRBeginREnd) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  const SBOVector<int, SBO_SIZE> sbo_small(vec.begin(), vec.end());
  int count = 0;
  for (auto iter = sbo_small.rbegin(); iter != sbo_small.rend();
       ++iter, ++count) {
    EXPECT_EQ(*iter, *(vec.rbegin() + count));
  }
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  vec = make_vector_sequence<LARGE_SIZE>();
  const SBOVector<int, SBO_SIZE> sbo_large(vec.begin(), vec.end());
  for (auto iter = sbo_large.rbegin(); iter != sbo_large.rend();
       ++iter, ++count) {
    EXPECT_EQ(*iter, *(vec.rbegin() + count));
  }
  EXPECT_EQ(count, LARGE_SIZE);
}

TYPED_TEST(SBOVector_, MustIterateViaCRBeginCREnd) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  size_t count{0};
  auto null_op = [&](const DataType&) { ++count; };
  const ContainerType small(SMALL_SIZE), large(LARGE_SIZE);
  for (auto iter = small.crbegin(); iter != small.crend(); ++iter)
    null_op(*iter);
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  for (auto iter = large.crbegin(); iter != large.crend(); ++iter)
    null_op(*iter);
  EXPECT_EQ(count, LARGE_SIZE);
}

TEST(ValueVerifiedSBOVector, MustIterateViaCRBeginCREnd) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  const SBOVector<int, SBO_SIZE> sbo_small(vec.begin(), vec.end());
  int count = 0;
  for (auto iter = sbo_small.crbegin(); iter != sbo_small.crend();
       ++iter, ++count) {
    EXPECT_EQ(*iter, *(vec.rbegin() + count));
  }
  EXPECT_EQ(count, SMALL_SIZE);
  count = 0;
  vec = make_vector_sequence<LARGE_SIZE>();
  const SBOVector<int, SBO_SIZE> sbo_large(vec.begin(), vec.end());
  for (auto iter = sbo_large.crbegin(); iter != sbo_large.crend();
       ++iter, ++count) {
    EXPECT_EQ(*iter, *(vec.rbegin() + count));
  }
  EXPECT_EQ(count, LARGE_SIZE);
}

TEST(ValueVerifiedSBOVector, MustAccessViaAt) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  for (size_t i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec.at(i), sbo.at(i));
  }
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (size_t i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec.at(i), sbo.at(i));
  }
}

TEST(ValueVerifiedSBOVector, MustAccessViaIndexOperator) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  for (size_t i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec[i], sbo[i]);
  }
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (size_t i = 0u; i < vec.size(); ++i) {
    EXPECT_EQ(vec[i], sbo[i]);
  }
}

TEST(ValueVerifiedSBOVector, MustAccessViaData) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  for (size_t i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec.data()[i], sbo.data()[i]);
  }
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (size_t i = 0u; i < vec.size(); ++i) {
    EXPECT_EQ(vec.data()[i], sbo.data()[i]);
  }
}

TEST(ValueVerifiedSBOVector, MustAccessViaCData) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  for (size_t i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec.data()[i], sbo.cdata()[i]);
  }
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  for (size_t i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(vec.data()[i], sbo.cdata()[i]);
  }
}

TEST(ValueVerifiedSBOVector, MustAccessViaFrontAndBack) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  EXPECT_EQ(vec.front(), sbo.front());
  EXPECT_EQ(vec.back(), sbo.back());
  vec = make_vector_sequence<LARGE_SIZE>();
  sbo.assign(vec.begin(), vec.end());
  EXPECT_EQ(vec.front(), sbo.front());
  EXPECT_EQ(vec.back(), sbo.back());
}

TYPED_TEST(SBOVector_, MustReportSize) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType empty{}, small(SMALL_SIZE), large(LARGE_SIZE);
  EXPECT_EQ(empty.size(), 0);
  EXPECT_EQ(small.size(), SMALL_SIZE);
  EXPECT_EQ(large.size(), LARGE_SIZE);
}
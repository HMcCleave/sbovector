#include "unittest_common.hpp"

// Unittests for insert, erase, push/pop _back, emplace, clear, resize methods

TYPED_TEST(CopyableSBOVector_, MustInsertSingleCopy) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  const DataType t{};
  ContainerType container;
  for (auto i = 1u; i <= LARGE_SIZE; ++i) {
    auto out = container.insert(container.begin() + (i / 10), t);
    EXPECT_EQ(container.size(), i);
    EXPECT_EQ(container.begin() + (i / 10), out);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustInsertSingleCopy) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  const DataType t{};
  ContainerType container(create_allocator());
  for (auto i = 1u; i <= LARGE_SIZE; ++i) {
    auto out = container.insert(container.begin() + (i / 10), t);
    EXPECT_EQ(container.size(), i);
    EXPECT_EQ(container.begin() + (i / 10), out);
    UseElements(container);
  }
}

TEST(ValueVerifiedSBOVector, MustInsertSingle) {
  std::vector<int> vec;
  SBOVector<int, SBO_SIZE> sbo;
  for (int i = 0; i < static_cast<int>(SBO_SIZE + 5); ++i) {
    vec.insert(vec.begin(), std::cref(i));
    sbo.insert(sbo.begin(), std::cref(i));
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_, MustInsertSingleMove) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  {
    {  // Small Container
      ContainerType small(SMALL_SIZE);
      auto out = small.insert(small.begin(), DataType());
      EXPECT_EQ(out, small.begin());
      EXPECT_EQ(small.size(), SMALL_SIZE + 1);
    }
    {  // Transitioning Container
      ContainerType full(SBO_SIZE);
      auto out = full.insert(full.begin(), DataType());
      EXPECT_EQ(full.size(), SBO_SIZE + 1);
      EXPECT_EQ(full.begin(), out);
    }
    {  // Large Container
      ContainerType large(LARGE_SIZE);
      auto out = large.insert(large.begin(), DataType());
      EXPECT_EQ(large.size(), LARGE_SIZE + 1);
      EXPECT_EQ(large.begin(), out);
    }
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustInsertSingleMove) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  ContainerType container(create_allocator());
  for (auto i = 1u; i <= LARGE_SIZE; ++i) {
    auto out = container.insert(container.begin() + (i / 10), DataType());
    EXPECT_EQ(container.size(), i);
    EXPECT_EQ(container.begin() + (i / 10), out);
    UseElements(container);
  }
}

TYPED_TEST(CopyableSBOVector_, MustInsertCountCopies) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  const DataType t{};
  ContainerType container;
  auto out = container.insert(container.begin(), 2, t);
  EXPECT_EQ(out, container.begin());
  EXPECT_EQ(container.size(), 2);
  out = container.insert(container.end(), SMALL_SIZE, t);
  EXPECT_EQ(out, container.begin() + 2);
  EXPECT_EQ(container.size(), SMALL_SIZE + 2);
  out = container.insert(container.begin() + 3, SBO_SIZE, t);
  EXPECT_EQ(out, container.begin() + 3);
  EXPECT_EQ(container.size(), SMALL_SIZE + SBO_SIZE + 2);
}

TEST_F(DataTypeOperationTrackingSBOVector, MustInsertCountCopies) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  const DataType t{};
  ContainerType container(create_allocator());
  auto out = container.insert(container.begin(), 2, t);
  EXPECT_EQ(out, container.begin());
  EXPECT_EQ(container.size(), 2);
  UseElements(container);
  out = container.insert(container.end(), SMALL_SIZE, t);
  EXPECT_EQ(out, container.begin() + 2);
  EXPECT_EQ(container.size(), SMALL_SIZE + 2);
  UseElements(container);
  out = container.insert(container.begin() + 3, SBO_SIZE, t);
  EXPECT_EQ(out, container.begin() + 3);
  EXPECT_EQ(container.size(), SMALL_SIZE + SBO_SIZE + 2);
  UseElements(container);
}

TEST(ValueVerifiedSBOVector, MustInsertCountCopies) {
  const int v{17};
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin(), 2, v);
    sbo.insert(sbo.cbegin(), 2, v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin() + 2, 5, v);
    sbo.insert(sbo.cbegin() + 2, 5, v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin() + 8, 15, v);
    sbo.insert(sbo.cbegin() + 8, 15, v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(CopyableSBOVector_, MustInsertRange) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  std::vector<DataType> range(3);
  ContainerType container;
  for (auto i = 1u; i <= (SBO_SIZE / range.size()) + 1; ++i) {
    auto iter = container.insert(container.begin(), range.begin(), range.end());
    EXPECT_EQ(iter, container.begin());
    EXPECT_EQ(container.size(), range.size() * i);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustInsertRange) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  std::vector<DataType> range(3);
  ContainerType container(create_allocator());
  for (auto i = 1u; i <= (SBO_SIZE / range.size()) + 1; ++i) {
    auto iter =
        container.insert(container.begin(), range.begin(), range.end());
    EXPECT_EQ(iter, container.begin());
    EXPECT_EQ(container.size(), range.size() * i);
    UseElements(container);
  }
}

TEST(ValueVerifiedSBOVector, MustInsertRange) {
  auto range = make_vector_sequence<3>();
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin(), range.begin(), range.end());
    sbo.insert(sbo.cbegin(), range.begin(), range.end());
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin(), range.begin(), range.end());
    sbo.insert(sbo.cbegin(), range.begin(), range.end());
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cend() - 1, range.begin(), range.end());
    sbo.insert(sbo.cend() - 1, range.begin(), range.end());
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(CopyableSBOVector_, MustInsertList) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  std::initializer_list<DataType> list{DataType(), DataType(), DataType()};
  ContainerType container;
  for (auto i = 1u; i <= (SBO_SIZE / list.size()) + 1; ++i) {
    auto iter =
        container.insert(container.begin(), list);
    EXPECT_EQ(iter, container.begin());
    EXPECT_EQ(container.size(), list.size() * i);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustInsertList) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  std::initializer_list<DataType> list{DataType(), DataType(), DataType()};
  ContainerType container(create_allocator());
  for (auto i = 1u; i <= (SBO_SIZE / list.size()) + 1; ++i) {
    auto iter = container.insert(container.begin(), list);
    EXPECT_EQ(iter, container.begin());
    EXPECT_EQ(container.size(), list.size() * i);
    UseElements(container);
  }
}

TEST(ValueVerifiedSBOVector, MustInsertList) {
  std::initializer_list<int> list = {5, 32};
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin(), list);
    sbo.insert(sbo.cbegin(), list);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin(), list);
    sbo.insert(sbo.cbegin(), list);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cend() - 1, list);
    sbo.insert(sbo.cend() - 1, list);
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_, MustEmplace) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType container;
  for (auto i = 1u; i <= SBO_SIZE + 5; ++i) {
    auto out = container.emplace(container.begin());
    EXPECT_EQ(out, container.begin());
    EXPECT_EQ(container.size(), i);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustEmplace) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType container(create_allocator());
  for (auto i = 1u; i <= SBO_SIZE + 5; ++i) {
    auto out = container.emplace(container.begin());
    EXPECT_EQ(out, container.begin());
    EXPECT_EQ(container.size(), i);
    UseElements(container);
  }
}

TEST(ValueVerifiedSBOVector, MustEmplace) {
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.emplace(vec.cbegin(), 17);
    sbo.emplace(sbo.cbegin(), 17);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.emplace(vec.cbegin() + 2, 17);
    sbo.emplace(sbo.cbegin() + 2, 17);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.emplace(vec.cbegin() + 8, 19);
    sbo.emplace(sbo.cbegin() + 8, 19);
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_, MustEraseSingleValue) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType container(LARGE_SIZE);
  auto ref_size = LARGE_SIZE;
  do {
    --ref_size;
    container.erase(container.begin());
    EXPECT_EQ(ref_size, container.size());
  } while (ref_size);
}

TEST_F(DataTypeOperationTrackingSBOVector, MustEraseSingleValue) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType container(LARGE_SIZE, create_allocator());
  auto ref_size = LARGE_SIZE;
  do {
    --ref_size;
    container.erase(container.begin());
    EXPECT_EQ(ref_size, container.size());
    UseElements(container);
  } while (ref_size);
}

TEST(ValueVerifiedSBOVector, MustEraseSingleValue) {
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.erase(vec.begin());
    sbo.erase(sbo.begin());
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE + 1>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.erase(vec.begin() + 5);
    sbo.erase(sbo.begin() + 5);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.erase(vec.end() - 1);
    sbo.erase(sbo.end() - 1);
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_, MustEraseRange) {
  using ContainerType = decltype(this->regular_container_);
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

TEST_F(DataTypeOperationTrackingSBOVector, MustEraseRange) {
  using ContainerType = decltype(this->regular_container_);
  {
    ContainerType small(SMALL_SIZE, create_allocator());
    small.erase(small.begin() + 1, small.end());
    EXPECT_EQ(small.size(), 1);
    UseElements(small);
  }
  {
    ContainerType shrinking(SBO_SIZE + 1, create_allocator());
    shrinking.erase(shrinking.begin() + 1, shrinking.end());
    EXPECT_EQ(shrinking.size(), 1);
    UseElements(shrinking);
  }
  {
    ContainerType large(LARGE_SIZE, create_allocator());
    large.erase(large.begin(), large.end() - 1);
    EXPECT_EQ(large.size(), 1);
    UseElements(large);
  }
}

TEST(ValueVerifiedSBOVector, MustEraseRange) {
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.erase(vec.begin(), vec.begin() + 1);
    sbo.erase(sbo.begin(), sbo.begin() + 1);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE + 1>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.erase(vec.begin() + 5, vec.end() - 1);
    sbo.erase(sbo.begin() + 5, sbo.end() - 1);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.erase(vec.end() - 5, vec.end());
    sbo.erase(sbo.end() - 5, sbo.end());
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(CopyableSBOVector_, MustPushBackCopy) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  const DataType t{};
  ContainerType container;
  for (auto i = 1u; i <= SBO_SIZE + 5; ++i) {
    container.push_back(t);
    EXPECT_EQ(container.size(), i);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustPushBackCopy) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  const DataType t;
  ContainerType container(create_allocator());
  for (auto i = 0u; i < LARGE_SIZE; ++i) {
    container.push_back(t);
    EXPECT_EQ(container.size(), i + 1);
    UseElements(container);
  }
}

TEST(ValueVerifiedSBOVector, MustPushBackCopy) {
  const int v{97};
  {
    std::vector<int> vec;
    SBOVector<int, SBO_SIZE> sbo;
    vec.push_back(v);
    sbo.push_back(v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.push_back(v);
    sbo.push_back(v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.push_back(v);
    sbo.push_back(v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.push_back(v);
    sbo.push_back(v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_, MustPushBackMove) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  ContainerType container;
  for (auto i = 0u; i < LARGE_SIZE; ++i) {
    container.push_back(DataType());
    EXPECT_EQ(container.size(), i + 1);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustPushBackMove) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  ContainerType container(create_allocator());
  for (auto i = 0u; i < LARGE_SIZE; ++i) {
    container.push_back(DataType());
    EXPECT_EQ(container.size(), i + 1);
    UseElements(container);
  }
}

TEST(ValueVerifiedSBOVector, MustPushBackMove) {
  {
    std::vector<int> vec;
    SBOVector<int, SBO_SIZE> sbo;
    vec.push_back(23);
    sbo.push_back(23);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.push_back(23);
    sbo.push_back(23);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.push_back(23);
    sbo.push_back(23);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.push_back(23);
    sbo.push_back(23);
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_, MustEmplaceBack) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType container;
  for (auto i = 0u; i < LARGE_SIZE; ++i) {
    container.emplace_back();
    EXPECT_EQ(container.size(), i + 1);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustEmplaceBack) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType container(create_allocator());
  for (auto i = 0u; i < LARGE_SIZE; ++i) {
    container.emplace_back();
    EXPECT_EQ(container.size(), i + 1);
    UseElements(container);
  }
}

TEST(ValueVerifiedSBOVector, MustEmplaceBack) {
  {
    std::vector<int> vec;
    SBOVector<int, SBO_SIZE> sbo;
    vec.emplace_back(23);
    sbo.emplace_back(23);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.emplace_back(23);
    sbo.emplace_back(23);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.emplace_back(23);
    sbo.emplace_back(23);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.emplace_back(23);
    sbo.emplace_back(23);
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_, MustPopBack) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType container(LARGE_SIZE);
  for (int count = LARGE_SIZE; count; --count) {
    container.pop_back();
    EXPECT_EQ(container.size(), count - 1);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustPopBack) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType container(LARGE_SIZE, create_allocator());
  for (int count = LARGE_SIZE; count; --count) {
    container.pop_back();
    EXPECT_EQ(container.size(), count - 1);
    UseElements(container);
  }
}

TEST(ValueVerifiedSBOVector, MustPopBack) {
  auto vec = make_vector_sequence<LARGE_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  while (vec.size()) {
    vec.pop_back();
    sbo.pop_back();
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_, MustResize) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType c(SMALL_SIZE);
  c.resize(SBO_SIZE);
  EXPECT_EQ(c.size(), SBO_SIZE);
  c.resize(LARGE_SIZE);
  EXPECT_EQ(c.size(), LARGE_SIZE);
  c.resize(SMALL_SIZE);
  EXPECT_EQ(c.size(), SMALL_SIZE);
}

TEST_F(DataTypeOperationTrackingSBOVector, MustResize) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = typename ContainerType::value_type;
  ContainerType c(SMALL_SIZE, create_allocator());
  c.resize(SBO_SIZE);
  EXPECT_EQ(c.size(), SBO_SIZE);
  UseElements(c);
  c.resize(LARGE_SIZE, DataType());
  EXPECT_EQ(c.size(), LARGE_SIZE);
  UseElements(c);
  c.resize(SMALL_SIZE);
  EXPECT_EQ(c.size(), SMALL_SIZE);
  UseElements(c);
  
}

TEST(ValueVerifiedSBOVector, MustResizeCount) {
  std::vector<int> vec;
  SBOVector<int, SBO_SIZE> sbo;
  vec.resize(SMALL_SIZE);
  sbo.resize(SMALL_SIZE);
  EXPECT_RANGE_EQ(vec, sbo);
  vec.resize(SBO_SIZE + 1);
  sbo.resize(SBO_SIZE + 1);
  EXPECT_RANGE_EQ(vec, sbo);
  vec.resize(LARGE_SIZE);
  sbo.resize(LARGE_SIZE);
  EXPECT_RANGE_EQ(vec, sbo);
  vec.resize(SMALL_SIZE);
  sbo.resize(SMALL_SIZE);
  EXPECT_RANGE_EQ(vec, sbo);
}

TEST(ValueVerifiedSBOVector, MustResizeCountValue) {
  std::vector<int> vec;
  SBOVector<int, SBO_SIZE> sbo;
  vec.resize(SMALL_SIZE, 3);
  sbo.resize(SMALL_SIZE, 3);
  EXPECT_RANGE_EQ(vec, sbo);
  vec.resize(SBO_SIZE + 1, 5);
  sbo.resize(SBO_SIZE + 1, 5);
  EXPECT_RANGE_EQ(vec, sbo);
  vec.resize(LARGE_SIZE, 7);
  sbo.resize(LARGE_SIZE, 7);
  EXPECT_RANGE_EQ(vec, sbo);
  vec.resize(SMALL_SIZE, 11);
  sbo.resize(SMALL_SIZE, 11);
  EXPECT_RANGE_EQ(vec, sbo);
}

TYPED_TEST(SBOVector_, MustClear) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType empty;
  ContainerType small(SMALL_SIZE);
  ContainerType large(LARGE_SIZE);
  empty.clear();
  small.clear();
  large.clear();
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(small.empty());
  EXPECT_TRUE(large.empty());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustClear) {
  using ContainerType = decltype(this->regular_container_);
  ContainerType empty(create_allocator());
  ContainerType small(SMALL_SIZE, create_allocator());
  ContainerType large(LARGE_SIZE, create_allocator());
  empty.clear();
  small.clear();
  large.clear();
  EXPECT_TRUE(empty.empty());
  EXPECT_TRUE(small.empty());
  EXPECT_TRUE(large.empty());
}
#include "unittest_common.hpp"

// Unittests for insert, erase, push/pop _back, emplace, clear, resize methods

TYPED_TEST(SBOVector_1, MustInsertSingleCopy) {
  const DataType t{};
  {  // Small Container
    ContainerType small(SMALL_SIZE);
    auto out = small.insert(small.begin(), t);
    EXPECT_EQ(out, small.begin());
    EXPECT_EQ(small.size(), SMALL_SIZE + 1);
  }
  {  // Transitioning Container
    ContainerType full(SBO_SIZE);
    full.insert(full.begin(), t);
    EXPECT_EQ(full.size(), SBO_SIZE + 1);
  }
  {  // Large Container
    ContainerType large(LARGE_SIZE);
    large.insert(large.begin(), t);
    EXPECT_EQ(large.size(), LARGE_SIZE + 1);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustInsertSingleCopy) {
  const DataType t{};
  {  // Small Container
    ContainerType small(SMALL_SIZE, create_allocator());
    auto out = small.insert(small.begin(), t);
    EXPECT_EQ(out, small.begin());
    EXPECT_EQ(small.size(), SMALL_SIZE + 1);
    UseElements(small);
  }
  {  // Transitioning Container
    ContainerType full(SBO_SIZE, create_allocator());
    full.insert(full.begin(),t);
    EXPECT_EQ(full.size(), SBO_SIZE + 1);
    UseElements(full);
  }
  {  // Large Container
    ContainerType large(LARGE_SIZE, create_allocator());
    large.insert(large.begin(), t);
    EXPECT_EQ(large.size(), LARGE_SIZE + 1);
    UseElements(large);
  }
}

TEST(ValueVerifiedSBOVector, MustInsertSingleCopy) {
  const int v{17};
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin(), v);
    sbo.insert(sbo.cbegin(), v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin() + 2, v);
    sbo.insert(sbo.cbegin() + 2, v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin() + 8, v);
    sbo.insert(sbo.cbegin() + 8, v);
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_1, MustInsertSingleMove) {
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
  {  // Small Container
    ContainerType small(SMALL_SIZE, create_allocator());
    auto out = small.insert(small.begin(), DataType());
    EXPECT_EQ(out, small.begin());
    EXPECT_EQ(small.size(), SMALL_SIZE + 1);
    UseElements(small);
  }
  {  // Transitioning Container
    ContainerType full(SBO_SIZE, create_allocator());
    auto out = full.insert(full.begin(), DataType());
    EXPECT_EQ(full.size(), SBO_SIZE + 1);
    EXPECT_EQ(full.begin(), out);
    UseElements(full);
  }
  {  // Large Container
    ContainerType large(LARGE_SIZE, create_allocator());
    auto out = large.insert(large.begin(), DataType());
    EXPECT_EQ(large.size(), LARGE_SIZE + 1);
    EXPECT_EQ(large.begin(), out);
    UseElements(large);
  }
}

TEST(ValueVerifiedSBOVector, MustInsertSingleMove) {
  {
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin(), 17);
    sbo.insert(sbo.cbegin(), 17);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<SBO_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin() + 2, 17);
    sbo.insert(sbo.cbegin() + 2, 17);
    EXPECT_RANGE_EQ(vec, sbo);
  }
  {
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    vec.insert(vec.cbegin() + 8, 19);
    sbo.insert(sbo.cbegin() + 8, 19);
    EXPECT_RANGE_EQ(vec, sbo);
  }
}

TYPED_TEST(SBOVector_1, MustInsertCountCopies) {
  const DataType t{};
  {  // Small Container
    ContainerType small(SMALL_SIZE);
    auto out = small.insert(small.begin(), 2,  t);
    EXPECT_EQ(out, small.begin());
    EXPECT_EQ(small.size(), SMALL_SIZE + 2);
  }
  {  // Transitioning Container
    ContainerType full(SBO_SIZE - 1);
    full.insert(full.begin(), 12,  t);
    EXPECT_EQ(full.size(), SBO_SIZE + 11);
  }
  {  // Large Container
    ContainerType large(LARGE_SIZE);
    large.insert(large.begin(), 15, t);
    EXPECT_EQ(large.size(), LARGE_SIZE + 15);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustInsertCountCopies) {
  {
    const DataType t{};
    {  // Small Container
      ContainerType small(SMALL_SIZE, create_allocator());
      auto out = small.insert(small.begin(), 2, t);
      EXPECT_EQ(out, small.begin());
      EXPECT_EQ(small.size(), SMALL_SIZE + 2);
    }
    {  // Transitioning Container
      ContainerType full(SBO_SIZE, create_allocator());
      full.insert(full.begin(), 7, t);
      EXPECT_EQ(full.size(), SBO_SIZE + 7);
    }
    {  // Large Container
      ContainerType large(LARGE_SIZE, create_allocator());
      large.insert(large.begin(), 15, t);
      EXPECT_EQ(large.size(), LARGE_SIZE + 15);
    }
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
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

TYPED_TEST(SBOVector_1, MustInsertRange) {
  std::vector<DataType> range(3);
  { 
    ContainerType small(SMALL_SIZE);
    auto iter = small.insert(small.begin(), range.begin(), range.end());
    EXPECT_EQ(small.size(), SMALL_SIZE + range.size());
    EXPECT_EQ(iter, small.begin());
  }
  { 
    ContainerType full(SBO_SIZE);
    auto iter = full.insert(full.begin() + 2, range.begin(), range.end());
    EXPECT_EQ(full.size(), SBO_SIZE + range.size());
    EXPECT_EQ(iter, full.begin() + 2);
  }
  {
    ContainerType large(LARGE_SIZE);
    auto iter = large.insert(large.begin() + 3, range.begin(), range.end());
    EXPECT_EQ(large.size(), LARGE_SIZE + range.size());
    EXPECT_EQ(iter, large.begin() + 3);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustInsertRange) {
  std::vector<DataType> range(3);
  {
    ContainerType small(SMALL_SIZE, create_allocator());
    auto iter = small.insert(small.begin(), range.begin(), range.end());
    EXPECT_EQ(small.size(), SMALL_SIZE + range.size());
    EXPECT_EQ(iter, small.begin());
    UseElements(small);
  }
  {
    ContainerType full(SBO_SIZE, create_allocator());
    auto iter = full.insert(full.begin() + 2, range.begin(), range.end());
    EXPECT_EQ(full.size(), SBO_SIZE + range.size());
    EXPECT_EQ(iter, full.begin() + 2);
    UseElements(full);
  }
  {
    ContainerType large(LARGE_SIZE, create_allocator());
    auto iter = large.insert(large.begin() + 3, range.begin(), range.end());
    EXPECT_EQ(large.size(), LARGE_SIZE + range.size());
    EXPECT_EQ(iter, large.begin() + 3);
    UseElements(large);
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

TYPED_TEST(SBOVector_1, MustInsertList) {
  std::initializer_list<DataType> list{DataType(), DataType(), DataType()};
  {
    ContainerType small(SMALL_SIZE);
    auto iter = small.insert(small.begin(), list);
    EXPECT_EQ(small.size(), SMALL_SIZE + list.size());
    EXPECT_EQ(iter, small.begin());
  }
  {
    ContainerType full(SBO_SIZE);
    auto iter = full.insert(full.begin() + 2, list);
    EXPECT_EQ(full.size(), SBO_SIZE + list.size());
    EXPECT_EQ(iter, full.begin() + 2);
  }
  {
    ContainerType large(LARGE_SIZE);
    auto iter = large.insert(large.begin() + 3, list);
    EXPECT_EQ(large.size(), LARGE_SIZE + list.size());
    EXPECT_EQ(iter, large.begin() + 3);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustInsertList) {
  std::initializer_list<DataType> list{DataType(), DataType(), DataType()};
  {
    ContainerType small(SMALL_SIZE, create_allocator());
    auto iter = small.insert(small.begin(), list);
    EXPECT_EQ(small.size(), SMALL_SIZE + list.size());
    EXPECT_EQ(iter, small.begin());
    UseElements(small);
  }
  {
    ContainerType full(SBO_SIZE, create_allocator());
    auto iter = full.insert(full.begin() + 2, list);
    EXPECT_EQ(full.size(), SBO_SIZE + list.size());
    EXPECT_EQ(iter, full.begin() + 2);
    UseElements(full);
  }
  {
    ContainerType large(LARGE_SIZE, create_allocator());
    auto iter = large.insert(large.begin() + 3, list);
    EXPECT_EQ(large.size(), LARGE_SIZE + list.size());
    EXPECT_EQ(iter, large.begin() + 3);
    UseElements(large);
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

TYPED_TEST(SBOVector_1, MustEmplace) {
  {
    {  // Small Container
      ContainerType small(SMALL_SIZE);
      auto out = small.emplace(small.begin(), DataType());
      EXPECT_EQ(out, small.begin());
      EXPECT_EQ(small.size(), SMALL_SIZE + 1);
    }
    {  // Transitioning Container
      ContainerType full(SBO_SIZE);
      full.emplace(full.begin(), DataType());
      EXPECT_EQ(full.size(), SBO_SIZE + 1);
    }
    {  // Large Container
      ContainerType large(LARGE_SIZE);
      large.emplace(large.begin(), DataType());
      EXPECT_EQ(large.size(), LARGE_SIZE + 1);
    }
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustEmplace) {
  {  // Small Container
    ContainerType small(SMALL_SIZE, create_allocator());
    auto out = small.emplace(small.begin(), DataType());
    EXPECT_EQ(out, small.begin());
    EXPECT_EQ(small.size(), SMALL_SIZE + 1);
    UseElements(small);
  }
  {  // Transitioning Container
    ContainerType full(SBO_SIZE, create_allocator());
    full.emplace(full.begin(), DataType());
    EXPECT_EQ(full.size(), SBO_SIZE + 1);
    UseElements(full);
  }
  {  // Large Container
    ContainerType large(LARGE_SIZE, create_allocator());
    large.emplace(large.begin(), DataType());
    EXPECT_EQ(large.size(), LARGE_SIZE + 1);
    UseElements(large);
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

TYPED_TEST(SBOVector_1, MustEraseSingleValue) {
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

TEST_F(DataTypeOperationTrackingSBOVector, MustEraseSingleValue) {
  {
    ContainerType small(SMALL_SIZE, create_allocator());
    small.erase(small.begin() + 1);
    EXPECT_EQ(small.size(), SMALL_SIZE - 1);
    UseElements(small);
  }
  {
    ContainerType shrinking(SBO_SIZE + 1, create_allocator());
    shrinking.erase(shrinking.begin());
    EXPECT_EQ(shrinking.size(), SBO_SIZE);
    UseElements(shrinking);
  }
  {
    ContainerType large(LARGE_SIZE, create_allocator());
    large.erase(large.end() - 1);
    EXPECT_EQ(large.size(), LARGE_SIZE - 1);
    UseElements(large);
  }
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

TYPED_TEST(SBOVector_1, MustEraseRange) {
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

TYPED_TEST(SBOVector_1, MustPushBackCopy) {
  const DataType t{};
  {
    ContainerType empty;
    empty.push_back(t);
    EXPECT_EQ(empty.size(), 1);
  }
  {
    ContainerType small(SMALL_SIZE);
    small.push_back(t);
    EXPECT_EQ(small.size(), SMALL_SIZE + 1);
  }
  {
    ContainerType full(SBO_SIZE);
    full.push_back(t);
    EXPECT_EQ(full.size(), SBO_SIZE + 1);
  }
  {
    ContainerType large(LARGE_SIZE);
    large.push_back(t);
    EXPECT_EQ(large.size(), LARGE_SIZE + 1);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustPushBackCopy) {
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

TYPED_TEST(SBOVector_1, MustPushBackMove) {
  {
    {
      ContainerType empty;
      empty.push_back(DataType());
      EXPECT_EQ(empty.size(), 1);
    }
    {
      ContainerType small(SMALL_SIZE);
      small.push_back(DataType());
      EXPECT_EQ(small.size(), SMALL_SIZE + 1);
    }
    {
      ContainerType full(SBO_SIZE);
      full.push_back(DataType());
      EXPECT_EQ(full.size(), SBO_SIZE + 1);
    }
    {
      ContainerType large(LARGE_SIZE);
      large.push_back(DataType());
      EXPECT_EQ(large.size(), LARGE_SIZE + 1);
    }
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustPushBackMove) {
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

TYPED_TEST(SBOVector_1, MustEmplaceBack) {
  {
    {
      ContainerType empty;
      empty.emplace_back();
      EXPECT_EQ(empty.size(), 1);
    }
    {
      ContainerType small(SMALL_SIZE);
      small.emplace_back();
      EXPECT_EQ(small.size(), SMALL_SIZE + 1);
    }
    {
      ContainerType full(SBO_SIZE);
      full.emplace_back();
      EXPECT_EQ(full.size(), SBO_SIZE + 1);
    }
    {
      ContainerType large(LARGE_SIZE);
      large.emplace_back();
      EXPECT_EQ(large.size(), LARGE_SIZE + 1);
    }
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustEmplaceBack) {
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

TYPED_TEST(SBOVector_1, MustPopBack) {
  ContainerType container(LARGE_SIZE);
  for (int count = LARGE_SIZE; count; --count) {
    container.pop_back();
    EXPECT_EQ(container.size(), count - 1);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustPopBack) {
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

TYPED_TEST(SBOVector_1, MustResize) {
  ContainerType c(SMALL_SIZE);
  c.resize(SBO_SIZE);
  EXPECT_EQ(c.size(), SBO_SIZE);
  c.resize(LARGE_SIZE, DataType());
  EXPECT_EQ(c.size(), LARGE_SIZE);
  c.resize(SMALL_SIZE);
  EXPECT_EQ(c.size(), SMALL_SIZE);
}

TEST_F(DataTypeOperationTrackingSBOVector, MustResize) {
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

TYPED_TEST(SBOVector_1, MustClear) {
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
#include "unittest_common.hpp"

// Unittests for operator=, assign methods
TYPED_TEST(SBOVector_, MustCopyAssignSmall) {
  const ContainerType original(SMALL_SIZE);
  ContainerType copy;
  copy = original;
  EXPECT_EQ(copy.size(), original.size());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustCopyAssignSmall) {
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

TEST(ValueVerifiedSBOVector, MustCopyAssignSmall) {
  auto vec = make_vector_sequence<SMALL_SIZE>();
  const SBOVector<int, SBO_SIZE> original{vec.begin(), vec.end()};
  SBOVector<int, SBO_SIZE> copy{};
  copy = original;
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustCopyAssignLarge) {
  const ContainerType original(LARGE_SIZE);
  ContainerType copy;
  copy = original;
  EXPECT_EQ(copy.size(), original.size());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustCopyAssignLarge) {
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

TEST(ValueVerifiedSBOVector, MustCopyAssignLarge) {
  auto vec = make_vector_sequence<LARGE_SIZE>();
  const SBOVector<int, SBO_SIZE> original(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> copy;
  copy = original;
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustCopyAssignAsymetric) {
  const ContainerType original(LARGE_SIZE);
  SBOVector<DataType, SMALL_SIZE, AllocatorType> copy;
  copy = original;
  EXPECT_EQ(original.size(), copy.size());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustCopyAssignAsymmetric) {
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

TEST(ValueVerifiedSBOVector, MustCopyAssignAsymmetric) {
  auto vec = make_vector_sequence<LARGE_SIZE>();
  const SBOVector<int, SBO_SIZE> original{vec.begin(), vec.end()};
  SBOVector<int, SMALL_SIZE, CustomAllocator<int>> copy;
  copy = original;
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustMoveAssignSmall) {
  ContainerType original(SMALL_SIZE);
  ContainerType copy;
  copy = std::move(original);
  EXPECT_EQ(copy.size(), SMALL_SIZE);
}

TEST_F(DataTypeOperationTrackingSBOVector, MustMoveAssignSmall) {
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

TEST(ValueVerifiedSBOVector, MustMoveAssignSmall) {
  auto vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> original(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> copy;
  copy = std::move(original);
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustMoveAssignLarge) {
  ContainerType original(LARGE_SIZE);
  ContainerType copy;
  copy = std::move(original);
  EXPECT_EQ(copy.size(), LARGE_SIZE);
}

TEST_F(DataTypeOperationTrackingSBOVector, MustMoveAssignLarge) {
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

TEST(ValueVerifiedSBOVector, MustMoveAssignLarge) {
  auto vec = make_vector_sequence<LARGE_SIZE>();
  SBOVector<int, SBO_SIZE> original{vec.begin(), vec.end()};
  SBOVector<int, SBO_SIZE> copy;
  copy = std::move(original);
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustMoveAssignAsymetric) {
  ContainerType original(LARGE_SIZE);
  SBOVector<DataType, SMALL_SIZE, AllocatorType> copy;
  copy = std::move(original);
  EXPECT_EQ(LARGE_SIZE, copy.size());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustMoveAssignAsymmetric) {
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

TEST(ValueVerifiedSBOVector, MustMoveAssignAsymmetric) {
  auto vec = make_vector_sequence<LARGE_SIZE>();
  SBOVector<int, SBO_SIZE> original(vec.begin(), vec.end());
  SBOVector<int, SMALL_SIZE, CustomAllocator<int>> copy{};
  copy = std::move(original);
  EXPECT_RANGE_EQ(copy, vec);
}

TYPED_TEST(SBOVector_, MustAssignFromInitializerList) {
  std::initializer_list<DataType> il{DataType(), DataType(), DataType()};
  ContainerType operated;
  ContainerType methoded;
  operated = il;
  methoded.assign(il);
  EXPECT_EQ(operated.size(), il.size());
  EXPECT_EQ(methoded.size(), il.size());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustAssignFromInitializerList) {
  {
    std::initializer_list<DataType> il{DataType(), DataType(), DataType()};
    ContainerType operated(create_allocator());
    ContainerType methoded(create_allocator());
    operated = il;
    methoded.assign(il);
    EXPECT_EQ(operated.size(), il.size());
    EXPECT_EQ(methoded.size(), il.size());
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
}

TEST(ValueVerifiedSBOVector, MustAssignFromInitializerList) {
  std::initializer_list<int> il{1, 2, 3, 4, 5};
  SBOVector<int, SBO_SIZE> operated;
  SBOVector<int, SBO_SIZE> methoded;
  operated = il;
  methoded.assign(il);
  EXPECT_RANGE_EQ(operated, il);
  EXPECT_RANGE_EQ(methoded, il);
}

TYPED_TEST(SBOVector_, MustAssignCountOfValues) {
  ContainerType container;

  // inline -> inline
  container.assign(SMALL_SIZE, DataType());
  EXPECT_EQ(container.size(), SMALL_SIZE);

  // inline -> external
  container.assign(LARGE_SIZE, DataType());
  EXPECT_EQ(container.size(), LARGE_SIZE);

  // external -> external
  container.assign(LARGE_SIZE * 2, DataType());
  EXPECT_EQ(container.size(), LARGE_SIZE * 2);

  // external -> inline
  container.assign(SMALL_SIZE, DataType());
  EXPECT_EQ(container.size(), SMALL_SIZE);
}

TEST_F(DataTypeOperationTrackingSBOVector, MustAssignCountOfValues) {
  {
    ContainerType container{create_allocator()};
    container.assign(SMALL_SIZE, DataType());
    EXPECT_EQ(container.size(), SMALL_SIZE);
    container.assign(LARGE_SIZE, DataType());
    EXPECT_EQ(container.size(), LARGE_SIZE);
    container.assign(LARGE_SIZE * 2, DataType());
    EXPECT_EQ(container.size(), LARGE_SIZE * 2);
    container.assign(SMALL_SIZE, DataType());
    EXPECT_EQ(container.size(), SMALL_SIZE);
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
}

TEST(ValueVerifiedSBOVector, MustAssignCountOfValues) {
  std::vector<int> vec;
  SBOVector<int, SBO_SIZE> sbo;

  vec.assign(SMALL_SIZE, 5);
  sbo.assign(SMALL_SIZE, 5);
  EXPECT_RANGE_EQ(vec, sbo);

  vec.assign(LARGE_SIZE, 7);
  sbo.assign(LARGE_SIZE, 7);
  EXPECT_RANGE_EQ(vec, sbo);

  vec.assign(LARGE_SIZE * 2, 11);
  sbo.assign(LARGE_SIZE * 2, 11);
  EXPECT_RANGE_EQ(vec, sbo);

  vec.assign(SMALL_SIZE, 13);
  sbo.assign(SMALL_SIZE, 13);
  EXPECT_RANGE_EQ(vec, sbo);
}

TYPED_TEST(SBOVector_, MustAssignRange) {
  std::vector<DataType> vec;
  ContainerType container;

  // inline -> inline
  vec.assign(SMALL_SIZE, DataType());
  container.assign(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), vec.size());

  // inline -> external
  vec.assign(LARGE_SIZE, DataType());
  container.assign(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), vec.size());

  // external -> external
  vec.assign(LARGE_SIZE * 2, DataType());
  container.assign(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), vec.size());

  // external -> inline
  vec.assign(SMALL_SIZE, DataType());
  container.assign(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), vec.size());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustAssignRange) {
  {
    std::vector<DataType> vec;
    ContainerType container(create_allocator());

    // inline -> inline
    vec.assign(SMALL_SIZE, DataType());
    container.assign(vec.begin(), vec.end());
    EXPECT_EQ(container.size(), vec.size());

    // inline -> external
    vec.assign(LARGE_SIZE, DataType());
    container.assign(vec.begin(), vec.end());
    EXPECT_EQ(container.size(), vec.size());

    // external -> external
    vec.assign(LARGE_SIZE * 2, DataType());
    container.assign(vec.begin(), vec.end());
    EXPECT_EQ(container.size(), vec.size());

    // external -> inline
    vec.assign(SMALL_SIZE, DataType());
    container.assign(vec.begin(), vec.end());
    EXPECT_EQ(container.size(), vec.size());
  }
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
}

TEST(ValueVerifiedSBOVector, MustAssignRange) {
  auto small_v = make_vector_sequence<SMALL_SIZE>();
  auto large_v = make_vector_sequence<LARGE_SIZE>();
  auto really_large_v = make_vector_sequence<LARGE_SIZE * 2>();

  SBOVector<int, SBO_SIZE> container;

  container.assign(small_v.begin(), small_v.end());
  EXPECT_RANGE_EQ(small_v, container);

  container.assign(large_v.begin(), large_v.end());
  EXPECT_RANGE_EQ(large_v, container);

  container.assign(really_large_v.begin(), really_large_v.end());
  EXPECT_RANGE_EQ(really_large_v, container);

  container.assign(small_v.begin(), small_v.end());
  EXPECT_RANGE_EQ(small_v, container);
}
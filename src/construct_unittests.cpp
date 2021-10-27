#include "unittest_common.hpp"

// Unit tests for the various Constructors of SBOVector

TYPED_TEST(SBOVector_, MustDefaultConstruct) {
  ContainerType container{};
  EXPECT_EQ(container.size(), 0);
  EXPECT_TRUE(container.empty());
}

TEST_F(OperationTrackingSBOVector, MustDefaultConstructWithAllocator) {
  {
    ContainerType container{create_allocator()};
    EXPECT_EQ(container.size(), 0);
    EXPECT_TRUE(container.empty());
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(), 0);
  EXPECT_EQ(totals_.allocs_, 0);
  EXPECT_EQ(totals_.frees_, 0);
}

TYPED_TEST(SBOVector_, MustConstructSmallNumberOfCopies) {
  ContainerType container(SMALL_SIZE);
  EXPECT_EQ(container.size(), SMALL_SIZE);
  EXPECT_EQ(container.capacity(), SBO_SIZE);
  EXPECT_FALSE(container.empty());
}

TEST_F(OperationTrackingSBOVector, MustConstructSmallNumberOfCopies) {
  {
    ContainerType container(SMALL_SIZE, create_allocator());
    EXPECT_EQ(container.size(), SMALL_SIZE);
    EXPECT_EQ(container.capacity(), SBO_SIZE);
    EXPECT_FALSE(container.empty());
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustConstructSmallNumberOfCopies) {
  SBOVector<int, SBO_SIZE> sbo(SMALL_SIZE);
  std::vector<int> vec(SMALL_SIZE);
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustConstructLargeNumberOfCopies) {
  ContainerType container(LARGE_SIZE);
  EXPECT_EQ(container.size(), LARGE_SIZE);
  EXPECT_GE(container.capacity(), LARGE_SIZE);
  EXPECT_FALSE(container.empty());
}

TEST_F(OperationTrackingSBOVector, MustConstructLargeNumberOfCopies) {
  {
    ContainerType container(LARGE_SIZE, create_allocator());
    EXPECT_EQ(container.size(), LARGE_SIZE);
    EXPECT_GE(container.capacity(), LARGE_SIZE);
    EXPECT_FALSE(container.empty());
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustConstructLargeNumberOfCopies) {
  SBOVector<int, SBO_SIZE> sbo(LARGE_SIZE);
  std::vector<int> vec(LARGE_SIZE);
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustConstructFromInitializerList) {
  std::initializer_list<DataType> list{
      DataType(), DataType(), DataType(), DataType(),
      DataType(), DataType(), DataType(), DataType(),
  };
  ContainerType container(list);
  EXPECT_EQ(container.size(), list.size());
  EXPECT_FALSE(container.empty());
}

TEST_F(OperationTrackingSBOVector, MustConstructFromInitializerList) {
  {
    std::initializer_list<DataType> list{
        DataType(), DataType(), DataType(), DataType(),
        DataType(), DataType(), DataType(), DataType(),
    };
    ContainerType container(list, create_allocator());
    EXPECT_EQ(container.size(), list.size());
    EXPECT_FALSE(container.empty());
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustConstructFromInitializerList) {
  std::initializer_list<int> list{1, 45, 6, 3, 5, 8, 19};
  SBOVector<int, SBO_SIZE> sbo(list);
  std::vector<int> vec(list);
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustConstructFromSmallRange) {
  std::vector<DataType> vec(SMALL_SIZE);
  ContainerType container(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), SMALL_SIZE);
}

TEST_F(OperationTrackingSBOVector, MustConstructFromSmallRange) {
  {
    std::vector<DataType> vec(SMALL_SIZE);
    ContainerType container(vec.begin(), vec.end(), create_allocator());
    EXPECT_EQ(container.size(), SMALL_SIZE);
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustConstructFromSmallRange) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustConstructFromLargeRange) {
  std::vector<DataType> vec(LARGE_SIZE);
  ContainerType container(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), LARGE_SIZE);
}

TEST_F(OperationTrackingSBOVector, MustConstructFromLargeRange) {
  {
    std::vector<DataType> vec(LARGE_SIZE);
    ContainerType container(vec.begin(), vec.end(), create_allocator());
    EXPECT_EQ(container.size(), LARGE_SIZE);
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustConstructFromLargeRange) {
  std::vector<int> vec = make_vector_sequence<LARGE_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustCopyConstructSmall) {
  const ContainerType old(SMALL_SIZE);
  ContainerType new_(old);
  EXPECT_EQ(new_.size(), SMALL_SIZE);
}

TEST_F(OperationTrackingSBOVector, MustCopyConstructSmall) {
  {
    const ContainerType old(SMALL_SIZE, create_allocator());
    ContainerType new_(old);
    EXPECT_EQ(new_.size(), SMALL_SIZE);
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustCopyConstructSmall) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  const SBOVector<int, SBO_SIZE> old(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> sbo(old);
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustCopyConstructLarge) {
  const ContainerType old(LARGE_SIZE);
  ContainerType new_(old);
  EXPECT_EQ(new_.size(), LARGE_SIZE);
}

TEST_F(OperationTrackingSBOVector, MustCopyConstructLarge) {
  {
    const ContainerType old(LARGE_SIZE, create_allocator());
    ContainerType new_(old);
    EXPECT_EQ(new_.size(), LARGE_SIZE);
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustCopyConstructLarge) {
  std::vector<int> vec = make_vector_sequence<LARGE_SIZE>();
  const SBOVector<int, SBO_SIZE> old(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> sbo(old);
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustCopyConstructAsymetric) {
  const SBOVector<DataType, SBO_SIZE + 10, AllocatorType> old(SBO_SIZE + 5);
  ContainerType new_(old);
  EXPECT_EQ(new_.size(), SBO_SIZE + 5);
}

TEST_F(OperationTrackingSBOVector, MustCopyConstructAsymetric) {
  {
    const SBOVector<DataType, SBO_SIZE + 10, AllocatorType> old(
        SBO_SIZE + 5, create_allocator());
    ContainerType new_(old);
    EXPECT_EQ(new_.size(), SBO_SIZE + 5);
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustCopyConstructAsymetric) {
  std::vector<int> vec = make_vector_sequence<SBO_SIZE + 5>();
  const SBOVector<int, SBO_SIZE + 10> old(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> sbo(old);
  EXPECT_RANGE_EQ(sbo, vec);
}

TEST(ValueVerifiedSBOVector, MustCopyConstructAsymetricAllocator) {
  std::vector<int> vec = make_vector_sequence<SBO_SIZE + 5>();
  const SBOVector<int, SBO_SIZE + 10, CustomAllocator<int>> old(vec.begin(),
                                                                vec.end());
  SBOVector<int, SBO_SIZE> sbo(old);
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustMoveConstructSmall) {
  ContainerType old(SMALL_SIZE);
  ContainerType new_(std::move(old));
  EXPECT_EQ(new_.size(), SMALL_SIZE);
}

TEST_F(OperationTrackingSBOVector, MustMoveConstructSmall) {
  {
    ContainerType old(SMALL_SIZE, create_allocator());
    ContainerType new_(std::move(old));
    EXPECT_EQ(new_.size(), SMALL_SIZE);
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustMoveConstructSmall) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> old(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> sbo(std::move(old));
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustMoveConstructLarge) {
  ContainerType old(LARGE_SIZE);
  ContainerType new_(std::move(old));
  EXPECT_EQ(new_.size(), LARGE_SIZE);
}

TEST_F(OperationTrackingSBOVector, MustMoveConstructLarge) {
  {
    ContainerType old(LARGE_SIZE, create_allocator());
    ContainerType new_(std::move(old));
    EXPECT_EQ(new_.size(), LARGE_SIZE);
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustMoveConstructLarge) {
  std::vector<int> vec = make_vector_sequence<LARGE_SIZE>();
  SBOVector<int, SBO_SIZE> old(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> sbo(std::move(old));
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustMoveConstructAsymetric) {
  SBOVector<DataType, SBO_SIZE + 10, AllocatorType> old(SBO_SIZE + 5);
  ContainerType new_(std::move(old));
  EXPECT_EQ(new_.size(), SBO_SIZE + 5);
}

TEST_F(OperationTrackingSBOVector, MustMoveConstructAsymetric) {
  {
    SBOVector<DataType, SBO_SIZE + 10, AllocatorType> old(SBO_SIZE + 5,
                                                          create_allocator());
    ContainerType new_(std::move(old));
    EXPECT_EQ(new_.size(), SBO_SIZE + 5);
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustMoveConstructAsymetric) {
  std::vector<int> vec = make_vector_sequence<SBO_SIZE + 5>();
  SBOVector<int, SBO_SIZE + 10> old(vec.begin(), vec.end());
  SBOVector<int, SBO_SIZE> sbo(std::move(old));
  EXPECT_RANGE_EQ(sbo, vec);
}

TEST(ValueVerifiedSBOVector, MustMoveConstructAsymetricAllocator) {
  std::vector<int> vec = make_vector_sequence<SBO_SIZE + 5>();
  SBOVector<int, SBO_SIZE + 10, CustomAllocator<int>> old(vec.begin(),
                                                          vec.end());
  SBOVector<int, SBO_SIZE> sbo(std::move(old));
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustIteratorConstructSmall) {
  std::vector<DataType> vec(SMALL_SIZE, DataType());
  ContainerType container(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), SMALL_SIZE);
  EXPECT_EQ(container.capacity(), SBO_SIZE);
  EXPECT_FALSE(container.empty());
}

TEST_F(OperationTrackingSBOVector, MustIteratorConstructSmall) {
  {
    std::vector<DataType> vec(SMALL_SIZE, DataType());
    ContainerType container(vec.begin(), vec.end(), create_allocator());
    EXPECT_EQ(container.size(), SMALL_SIZE);
    EXPECT_EQ(container.capacity(), SBO_SIZE);
    EXPECT_FALSE(container.empty());
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustIteratorConstructSmall) {
  std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(SBOVector_, MustIteratorConstructLarge) {
  std::vector<DataType> vec(LARGE_SIZE, DataType());
  ContainerType container(vec.begin(), vec.end());
  EXPECT_EQ(container.size(), LARGE_SIZE);
  EXPECT_GE(container.capacity(), LARGE_SIZE);
  EXPECT_FALSE(container.empty());
}

TEST_F(OperationTrackingSBOVector, MustIteratorConstructLarge) {
  {
    std::vector<DataType> vec(LARGE_SIZE, DataType());
    ContainerType container(vec.begin(), vec.end(), create_allocator());
    EXPECT_EQ(container.size(), LARGE_SIZE);
    EXPECT_GE(container.capacity(), LARGE_SIZE);
    EXPECT_FALSE(container.empty());
  }
  EXPECT_EQ(OperationCounter::TOTALS.constructs(),
            OperationCounter::TOTALS.destructs());
  EXPECT_EQ(totals_.allocs_, totals_.frees_);
}

TEST(ValueVerifiedSBOVector, MustIteratorConstructLarge) {
  std::vector<int> vec = make_vector_sequence<LARGE_SIZE>();
  SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
  EXPECT_RANGE_EQ(sbo, vec);
}
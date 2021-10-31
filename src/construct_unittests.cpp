#include "unittest_common.hpp"

// Unit tests for the various Constructors of SBOVector

TYPED_TEST(SBOVector_, MustDefaultConstruct) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  ContainerType container{};
  EXPECT_EQ(container.size(), 0);
  EXPECT_TRUE(container.empty());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustDefaultConstructWithAllocator) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  ContainerType container{create_allocator()};
  EXPECT_EQ(container.size(), 0);
  EXPECT_TRUE(container.empty());
}

TYPED_TEST(SBOVector_, MustConstructCount) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  ContainerType small(SMALL_SIZE);
  EXPECT_EQ(small.size(), SMALL_SIZE);
  EXPECT_EQ(small.capacity(), SBO_SIZE);
  EXPECT_FALSE(small.empty());
  ContainerType large(LARGE_SIZE);
  EXPECT_EQ(large.size(), LARGE_SIZE);
  EXPECT_GE(large.capacity(), LARGE_SIZE);
  EXPECT_FALSE(large.empty());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustConstructCount) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  ContainerType small(SMALL_SIZE, create_allocator());
  EXPECT_EQ(small.size(), SMALL_SIZE);
  EXPECT_EQ(small.capacity(), SBO_SIZE);
  EXPECT_FALSE(small.empty());
  UseElements(small);
  ContainerType large(LARGE_SIZE, create_allocator());
  EXPECT_EQ(large.size(), LARGE_SIZE);
  EXPECT_GE(large.capacity(), LARGE_SIZE);
  EXPECT_FALSE(large.empty());
  UseElements(large);
}

TEST(ValueVerifiedSBOVector, MustConstructCount) {
  {
    SBOVector<int, SBO_SIZE> sbo(SMALL_SIZE);
    std::vector<int> vec(SMALL_SIZE);
    EXPECT_RANGE_EQ(sbo, vec);
  }
  {
    SBOVector<int, SBO_SIZE> sbo(LARGE_SIZE);
    std::vector<int> vec(LARGE_SIZE);
    EXPECT_RANGE_EQ(sbo, vec);
  }
}

TEST(ValueVerifiedSBOVector, MustConstructCountValue) {
  {
    SBOVector<int, SBO_SIZE> sbo(SMALL_SIZE, 5);
    std::vector<int> vec(SMALL_SIZE, 5);
    EXPECT_RANGE_EQ(sbo, vec);
  }
  {
    SBOVector<int, SBO_SIZE> sbo(LARGE_SIZE, -13);
    std::vector<int> vec(LARGE_SIZE, -13);
    EXPECT_RANGE_EQ(sbo, vec);
  }
}

TYPED_TEST(CopyableSBOVector_, MustConstructFromInitializerList) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  std::initializer_list<DataType> list{
      DataType(), DataType(), DataType(), DataType(),
      DataType(), DataType(), DataType(), DataType(),
  };
  ContainerType container(list);
  EXPECT_EQ(container.size(), list.size());
  EXPECT_FALSE(container.empty());
}

TEST_F(DataTypeOperationTrackingSBOVector, MustConstructFromInitializerList) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  std::initializer_list<DataType> list{
      DataType(), DataType(), DataType(), DataType(),
      DataType(), DataType(), DataType(), DataType(),
  };
  ContainerType container(list, create_allocator());
  EXPECT_EQ(container.size(), list.size());
  EXPECT_FALSE(container.empty());
  UseElements(container);
}

TEST(ValueVerifiedSBOVector, MustConstructFromInitializerList) {
  std::initializer_list<int> list{1, 45, 6, 3, 5, 8, 19};
  SBOVector<int, SBO_SIZE> sbo(list);
  std::vector<int> vec(list);
  EXPECT_RANGE_EQ(sbo, vec);
}

TYPED_TEST(CopyableSBOVector_, MustConstructFromRange) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  {
    std::vector<DataType> vec(SMALL_SIZE);
    ContainerType container(vec.begin(), vec.end());
    EXPECT_EQ(container.size(), SMALL_SIZE);
  }
  {
    std::vector<DataType> vec(LARGE_SIZE);
    ContainerType container(vec.begin(), vec.end());
    EXPECT_EQ(container.size(), LARGE_SIZE);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustConstructFromRange) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  {
    std::vector<DataType> vec(SMALL_SIZE);
    ContainerType container(vec.begin(), vec.end(), create_allocator());
    EXPECT_EQ(container.size(), SMALL_SIZE);
    UseElements(container);
  }
  {
    std::vector<DataType> vec(LARGE_SIZE);
    ContainerType container(vec.begin(), vec.end(), create_allocator());
    EXPECT_EQ(container.size(), LARGE_SIZE);
    UseElements(container);
  }
}

TEST(ValueVerifiedSBOVector, MustConstructFromRange) {
  {
    std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    EXPECT_RANGE_EQ(sbo, vec);
  }
  {
    std::vector<int> vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> sbo(vec.begin(), vec.end());
    EXPECT_RANGE_EQ(sbo, vec);
  }
}

TYPED_TEST(CopyableSBOVector_, MustCopyConstruct) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  {
    const ContainerType old(SMALL_SIZE);
    ContainerType new_(old);
    EXPECT_EQ(new_.size(), SMALL_SIZE);
  }
  {
    const ContainerType old(LARGE_SIZE);
    ContainerType new_(old);
    EXPECT_EQ(new_.size(), LARGE_SIZE);
  }
  {
    const SBOVector<DataType, SBO_SIZE + 10, AllocatorType> old(SBO_SIZE + 5);
    ContainerType new_(old);
    EXPECT_EQ(new_.size(), SBO_SIZE + 5);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustCopyConstruct) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  {
    const ContainerType old(SMALL_SIZE, create_allocator());
    ContainerType new_(old);
    EXPECT_EQ(new_.size(), SMALL_SIZE);
    UseElements(old);
    UseElements(new_);
  }
  {
    const ContainerType old(LARGE_SIZE, create_allocator());
    ContainerType new_(old);
    EXPECT_EQ(new_.size(), LARGE_SIZE);
    UseElements(old);
    UseElements(new_);
  }
  {
    const SBOVector<DataType, SBO_SIZE + 10, AllocatorType> old(
        SBO_SIZE + 5, create_allocator());
    ContainerType new_(old);
    EXPECT_EQ(new_.size(), SBO_SIZE + 5);
    UseElements(old);
    UseElements(new_);
  }
}

TEST(ValueVerifiedSBOVector, MustCopyConstruct) {
  {
    std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
    const SBOVector<int, SBO_SIZE> old(vec.begin(), vec.end());
    SBOVector<int, SBO_SIZE> sbo(old);
    EXPECT_RANGE_EQ(sbo, vec);
  }
  {
    std::vector<int> vec = make_vector_sequence<LARGE_SIZE>();
    const SBOVector<int, SBO_SIZE> old(vec.begin(), vec.end());
    SBOVector<int, SBO_SIZE> sbo(old);
    EXPECT_RANGE_EQ(sbo, vec);
  }
  {
    std::vector<int> vec = make_vector_sequence<SBO_SIZE + 5>();
    const SBOVector<int, SBO_SIZE + 10> old(vec.begin(), vec.end());
    SBOVector<int, SBO_SIZE> sbo(old);
    EXPECT_RANGE_EQ(sbo, vec);
  }
}

TYPED_TEST(SBOVector_, MustMoveConstruct) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  {
    ContainerType old(SMALL_SIZE);
    ContainerType new_(std::move(old));
    EXPECT_EQ(new_.size(), SMALL_SIZE);
  }
  {
    ContainerType old(LARGE_SIZE);
    ContainerType new_(std::move(old));
    EXPECT_EQ(new_.size(), LARGE_SIZE);
  }
  {
    SBOVector<DataType, SBO_SIZE + 10, AllocatorType> old(SBO_SIZE + 5);
    ContainerType new_(std::move(old));
    EXPECT_EQ(new_.size(), SBO_SIZE + 5);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustMoveConstruct) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  {
    ContainerType old(SMALL_SIZE, create_allocator());
    ContainerType new_(std::move(old));
    EXPECT_EQ(new_.size(), SMALL_SIZE);
    UseElements(new_);
  }
  {
    ContainerType old(LARGE_SIZE, create_allocator());
    ContainerType new_(std::move(old));
    EXPECT_EQ(new_.size(), LARGE_SIZE);
    UseElements(new_);
  }
  {
    SBOVector<DataType, SBO_SIZE + 10, AllocatorType> old(SBO_SIZE + 5,
                                                          create_allocator());
    ContainerType new_(std::move(old));
    EXPECT_EQ(new_.size(), SBO_SIZE + 5);
    UseElements(new_);
  }
}

TEST(ValueVerifiedSBOVector, MustMoveConstruct) {
  {
    std::vector<int> vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> old(vec.begin(), vec.end());
    SBOVector<int, SBO_SIZE> sbo(std::move(old));
    EXPECT_RANGE_EQ(sbo, vec);
  }
  {
    std::vector<int> vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> old(vec.begin(), vec.end());
    SBOVector<int, SBO_SIZE> sbo(std::move(old));
    EXPECT_RANGE_EQ(sbo, vec);
  }
  {
    std::vector<int> vec = make_vector_sequence<SBO_SIZE + 5>();
    SBOVector<int, SBO_SIZE + 10> old(vec.begin(), vec.end());
    SBOVector<int, SBO_SIZE> sbo(std::move(old));
    EXPECT_RANGE_EQ(sbo, vec);
  }
}
TEST(ValueVerifiedSBOVector, MustMoveConstructAsymetricAllocator) {
  std::vector<int> vec = make_vector_sequence<SBO_SIZE + 5>();
  SBOVector<int, SBO_SIZE + 10, CustomAllocator<int>> old(vec.begin(),
                                                          vec.end());
  SBOVector<int, SBO_SIZE> sbo(std::move(old));
  EXPECT_RANGE_EQ(sbo, vec);
}

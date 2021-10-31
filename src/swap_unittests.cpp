#include "unittest_common.hpp"

// Unittests for swap methods

TYPED_TEST(SBOVector_, MustSwap) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  { 
    ContainerType a;
    ContainerType b(SMALL_SIZE);
    ContainerType c(LARGE_SIZE);
    b.swap(a); // empty <> inline
    EXPECT_EQ(b.size(), 0);
    c.swap(b); // empty <> external
    EXPECT_EQ(a.size(), SMALL_SIZE);
    EXPECT_EQ(b.size(), LARGE_SIZE);
    EXPECT_EQ(c.size(), 0);
    a.swap(b); // inline <> external
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustSwap) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  ContainerType a(create_allocator());
  ContainerType b(SMALL_SIZE, create_allocator());
  ContainerType c(LARGE_SIZE, create_allocator());
  b.swap(a); // empty <> inline
  EXPECT_EQ(b.size(), 0);
  UseElements(a);
  c.swap(b); // empty <> external
  EXPECT_EQ(a.size(), SMALL_SIZE);
  EXPECT_EQ(b.size(), LARGE_SIZE);
  EXPECT_EQ(c.size(), 0);
  UseElements(b);
  a.swap(b); // inline <> external
  UseElements(a);
  UseElements(b);
}

TEST(ValueVerifiedSBOVector, MustSwap) {
  { // empty <> inline
    auto vec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> a{}, b(vec.begin(), vec.end());
    a.swap(b); 
    EXPECT_RANGE_EQ(a, vec);
  }
  {  // empty <> external
    auto vec = make_vector_sequence<LARGE_SIZE>();
    SBOVector<int, SBO_SIZE> a{}, b(vec.begin(), vec.end());
    a.swap(b);
    EXPECT_RANGE_EQ(a, vec);
  }
  {  // inline <> external
    auto avec = make_vector_sequence<LARGE_SIZE>();
    auto bvec = make_vector_sequence<SMALL_SIZE>();
    SBOVector<int, SBO_SIZE> a(avec.begin(), avec.end()), b(bvec.begin(), bvec.end());
    a.swap(b);
    EXPECT_RANGE_EQ(a, bvec);
    EXPECT_RANGE_EQ(b, avec);
  }
}

TYPED_TEST(SBOVector_, MustSwapAsymmetric) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  { // inline to insufficient inline
    ContainerType a(SBO_SIZE);
    SBOVector<DataType, SMALL_SIZE, AllocatorType> b(SMALL_SIZE);
    a.swap(b);
    EXPECT_EQ(a.size(), SMALL_SIZE);
    EXPECT_EQ(b.size(), SBO_SIZE);
  }
  { // external to sufficent inline
    ContainerType a(SBO_SIZE);
    SBOVector<DataType, SMALL_SIZE, AllocatorType> b(SBO_SIZE);
    a.swap(b);
    EXPECT_EQ(a.size(), SBO_SIZE);
    EXPECT_EQ(b.size(), SBO_SIZE);
  }
  { // external to external
    ContainerType a(LARGE_SIZE);
    SBOVector<DataType, SMALL_SIZE, AllocatorType> b(LARGE_SIZE);
    a.swap(b);
    EXPECT_EQ(a.size(), LARGE_SIZE);
    EXPECT_EQ(b.size(), LARGE_SIZE);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustSwapAsymmetric) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  {  // inline to insufficient inline
    ContainerType a(SBO_SIZE, create_allocator());
    SBOVector<DataType, SMALL_SIZE, AllocatorType> b(SMALL_SIZE,
                                                      create_allocator());
    a.swap(b);
    EXPECT_EQ(a.size(), SMALL_SIZE);
    EXPECT_EQ(b.size(), SBO_SIZE);
    UseElements(a);
    UseElements(b);
  }
  {  // external to sufficent inline
    ContainerType a(SBO_SIZE, create_allocator());
    SBOVector<DataType, SMALL_SIZE, AllocatorType> b(SBO_SIZE,
                                                      create_allocator());
    a.swap(b);
    EXPECT_EQ(a.size(), SBO_SIZE);
    EXPECT_EQ(b.size(), SBO_SIZE);
    UseElements(a);
    UseElements(b);
  }
  {  // external to external
    ContainerType a(LARGE_SIZE, create_allocator());
    SBOVector<DataType, SMALL_SIZE, AllocatorType> b(LARGE_SIZE,
                                                      create_allocator());
    a.swap(b);
    EXPECT_EQ(a.size(), LARGE_SIZE);
    EXPECT_EQ(b.size(), LARGE_SIZE);
    UseElements(a);
    UseElements(b);
  }
}

TEST(ValueVerifiedSBOVector, MustSwapAsymmetric) {
  auto small_vec = make_vector_sequence<SMALL_SIZE>();
  auto sbos_vec = make_vector_sequence<SBO_SIZE>();
  auto large_vec = make_vector_sequence<LARGE_SIZE>();
  {  // inline to insufficient inline
    SBOVector<int, SBO_SIZE> a(sbos_vec.begin(), sbos_vec.end());
    SBOVector<int, SMALL_SIZE> b(small_vec.begin(), small_vec.end());
    a.swap(b);
    EXPECT_RANGE_EQ(a, small_vec);
    EXPECT_RANGE_EQ(b, sbos_vec);
  }
  {  // external to sufficent inline
    SBOVector<int, SBO_SIZE> a(sbos_vec.begin(), sbos_vec.end());
    SBOVector<int, SMALL_SIZE> b(sbos_vec.begin(), sbos_vec.end());
    a.swap(b);
    EXPECT_RANGE_EQ(a, sbos_vec);
    EXPECT_RANGE_EQ(b, sbos_vec);
  }
  {  // external to external
    SBOVector<int, SBO_SIZE> a(large_vec.begin(), large_vec.end());
    SBOVector<int, SMALL_SIZE> b(large_vec.begin(), large_vec.end());
    a.swap(b);
    EXPECT_RANGE_EQ(a, large_vec);
    EXPECT_RANGE_EQ(b, large_vec);
  }
}

TEST_F(DataTypeOperationTrackingSBOVector, MustSwapAsymmetricAllocators) {
  using ContainerType = decltype(this->regular_container_);
  using DataType = ContainerType::value_type;
  using AllocatorType = ContainerType::allocator_type;
  {  // inline to insufficient inline
    ContainerType a(SBO_SIZE, create_allocator());
    SBOVector<DataType, SMALL_SIZE> b(SMALL_SIZE);
    a.swap(b);
    EXPECT_EQ(a.size(), SMALL_SIZE);
    EXPECT_EQ(b.size(), SBO_SIZE);
    UseElements(a);
    UseElements(b);
  }
  {  // external to sufficent inline
    ContainerType a(SBO_SIZE, create_allocator());
    SBOVector<DataType, SMALL_SIZE> b(SBO_SIZE);
    a.swap(b);
    EXPECT_EQ(a.size(), SBO_SIZE);
    EXPECT_EQ(b.size(), SBO_SIZE);
    UseElements(a);
    UseElements(b);
  }
  {  // external to external
    ContainerType a(LARGE_SIZE, create_allocator());
    SBOVector<DataType, SMALL_SIZE> b(LARGE_SIZE);
    a.swap(b);
    EXPECT_EQ(a.size(), LARGE_SIZE);
    EXPECT_EQ(b.size(), LARGE_SIZE);
    UseElements(a);
    UseElements(b);
  }
}
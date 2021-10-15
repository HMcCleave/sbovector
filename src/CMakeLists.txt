add_executable(Hello main.cpp)

add_executable(TestBM test_benchmark.cpp)
target_link_libraries(TestBM PRIVATE BenchmarkSettings)

CREATE_UNITTEST(sample SOURCES test_unittest.cpp)
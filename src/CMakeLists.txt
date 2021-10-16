add_executable(Hello main.cpp)

add_executable(benchmarks benchmarks.cpp)
target_link_libraries(benchmarks PRIVATE BenchmarkSettings)

CREATE_UNITTEST(all SOURCES unittests.cpp)
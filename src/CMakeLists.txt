add_executable(benchmarks benchmarks.cpp sbovector.hpp)
target_link_libraries(benchmarks PRIVATE BenchmarkSettings)

CREATE_UNITTEST(all SOURCES unittests.cpp sbovector.hpp)
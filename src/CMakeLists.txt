add_executable(benchmarks benchmarks.cpp sbovector.hpp)
target_link_libraries(benchmarks PRIVATE BenchmarkSettings)

CREATE_UNITTEST(all 
  SOURCES 
    access_unittests.cpp
    assign_unittests.cpp
    capacity_unittests.cpp
    construct_unittests.cpp
    modify_unittests.cpp
    sbovector.hpp
    swap_unittests.cpp
    unittest_common.cpp unittest_common.hpp
)
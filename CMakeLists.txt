cmake_minimum_required(VERSION 3.14)

project(SBOVector)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

include(cmake/compiler_warnings.cmake)
include(cmake/general_settings.cmake)

SET(CONAN_REQUIRES
  benchmark/1.5.0
  gtest/1.10.0
)
SET(CONAN_OPTIONS
  gtest:build_gmock=False
)

include (cmake/conan.cmake)
run_conan()

add_subdirectory(src)


include(CTest)
enable_testing()

# Create interface libraries to provide settings for common project types

# General C++ 17 Settings
add_library(GeneralSettings INTERFACE)
target_compile_features(GeneralSettings INTERFACE cxx_std_17)
target_compile_options(GeneralSettings INTERFACE ${CPP_WARNINGS})
target_include_directories(GeneralSettings INTERFACE
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/include
  ${PROJECT_BINARY_DIR}/include
)

# Unit Testing Interfaces
add_library(UnitTestSettings INTERFACE)
target_link_libraries(UnitTestSettings INTERFACE CONAN_PKG::gtest GeneralSettings)

# Benchmarking Interface
add_library(BenchmarkSettings INTERFACE)
target_link_libraries(BenchmarkSettings INTERFACE CONAN_PKG::benchmark GeneralSettings)

function(CREATE_UNITTEST TEST_NAME)
  set(options DISABLE)
  set(oneValueArgs DUMMY)
  set(multiValueArgs SOURCES LIBS)
  cmake_parse_arguments(${TEST_NAME} "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
  include(GoogleTest)
  add_executable(${TEST_NAME}_unittest ${${TEST_NAME}_SOURCES})
  target_link_libraries(${TEST_NAME}_unittest PRIVATE ${${TEST_NAME}_LIBS} UnitTestSettings)
  gtest_discover_tests(${TEST_NAME}_unittest)
endfunction()
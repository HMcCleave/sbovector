macro(run_conan)
# Download automatically, you can also just copy the conan.cmake file
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(
    STATUS
      "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.16.1/conan.cmake"
       "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_autodetect(settings)

conan_cmake_run(
  REQUIRES
    ${CONAN_REQUIRES}
  OPTIONS
    ${CONAN_OPTIONS}
  CMAKE_TARGETS
  BASIC_SETUP
  BUILD
    missing
  SETTINGS
    ${settings}
)

endmacro()
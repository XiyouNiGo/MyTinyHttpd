cmake_minimum_required(VERSION 3.5)

project(mytinyhttpd)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Compiler flags
set(CXX_FLAGS
    -g
    # -DVALGRIND -DMUDUO_STD_STRING
    -Wall
    -Wextra
    # -m32
    -Werror
    # GoogleTest is not compatible with this option -Wconversion
    -Wno-unused-parameter
    -Wold-style-cast
    -Woverloaded-virtual
    -Wpointer-arith
    -Wshadow
    -Wwrite-strings
    -march=native
    # -MMD -std=c++0x
    -rdynamic)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wthread-safety HAVE_CLANG_THREAD_SAFETY)

# Options
option(MYTINYHTTPD_BUILD_TESTS "Build MyTinyHttpd's unit tests" ON)
option(MYTINYHTTPD_INSTALL "Install MyTinyHttpd's header and library" ON)
# option(MYTINYHTTPD_BUILD_BENCHMARKS "Build MyTinyHttpd's benchmarks" ON)

include_directories(${PROJECT_SOURCE_DIR})

# Base
aux_source_directory(${PROJECT_NAME}/base BASE_SOURCES)
add_library(mytinyhttpd_base STATIC ${BASE_SOURCES})
target_link_libraries(mytinyhttpd_base PUBLIC mytinyhttpd_utils pthread)

# Net
aux_source_directory(${PROJECT_NAME}/net NET_SOURCES)
add_library(mytinyhttpd_net STATIC ${NET_SOURCES})
target_link_libraries(mytinyhttpd_net PUBLIC mytinyhttpd_base pthread)

# Utils
aux_source_directory(${PROJECT_NAME}/utils UTILS_SOURCES)
add_library(mytinyhttpd_utils STATIC ${UTILS_SOURCES})

# Httpd
add_executable(mytinyhttpd mytinyhttpd/httpd/mytinyhttpd.cc)
target_link_libraries(mytinyhttpd PUBLIC mytinyhttpd_base mytinyhttpd_net
                                         mytinyhttpd_utils)
if(HAVE_CLANG_THREAD_SAFETY)
  target_compile_options(mytinyhttpd PUBLIC -Werror -Wthread-safety)
endif(HAVE_CLANG_THREAD_SAFETY)

# Test
if(MYTINYHTTPD_BUILD_TESTS)
  enable_testing()
  # This project is tested using GoogleTest.
  add_subdirectory("third_party/googletest")

  function(mytinyhttpd_test test_file)
    get_filename_component(test_target_name "${test_file}" NAME_WE)
    add_executable("${test_target_name}" "")
    target_sources(
      "${test_target_name}"
      PRIVATE "mytinyhttpd/utils/testutil.cc" "mytinyhttpd/utils/testutil.h"
              "${test_file}")
    target_link_libraries(
      "${test_target_name}"
      mytinyhttpd_base
      mytinyhttpd_net
      mytinyhttpd_utils
      gtest
      gtest_main
      pthread)
    add_test(NAME "${test_target_name}" COMMAND "${test_target_name}")
  endfunction(mytinyhttpd_test)

  mytinyhttpd_test("mytinyhttpd/utils/gtest_test.cc")
endif(MYTINYHTTPD_BUILD_TESTS)

# Install
if(MYTINYHTTPD_INSTALL)
  install(TARGETS mytinyhttpd_base)
  file(GLOB BASE_HEADERS ${PROJECT_NAME}/base/*.h)
  install(FILES ${BASE_HEADERS} DESTINATION include/${PROJECT_NAME}/base)

  install(TARGETS mytinyhttpd_net)
  file(GLOB NET_HEADERS ${PROJECT_NAME}/net/*.h)
  install(FILES ${NET_HEADERS} DESTINATION include/${PROJECT_NAME}/net)

  install(TARGETS mytinyhttpd_utils)
  file(GLOB UTILS_HEADERS ${PROJECT_NAME}/utils/*.h)
  install(FILES ${UTILS_HEADERS} DESTINATION include/${PROJECT_NAME}/utils)

  file(GLOB UTILS_HEADERS ${PROJECT_NAME}/httpd/*.h)
  install(FILES ${UTILS_HEADERS} DESTINATION include/${PROJECT_NAME}/httpd)
endif(MYTINYHTTPD_INSTALL)

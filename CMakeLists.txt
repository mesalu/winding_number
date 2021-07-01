project(winding_number_homework LANGUAGES CXX)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)

include(GoogleTest)
enable_testing()

# the guts of the library that computes winding number
set(WINDING_NUMBER_INC
  include/poly_io.hpp
  include/winding.hpp
)

set(WINDING_NUMBER_SRC
  src/poly_io.cpp
  src/winding.cpp
)

add_library(winding_lib STATIC ${WINDING_NUMBER_SRC} ${WINDING_NUMBER_INC})
target_include_directories(winding_lib PUBLIC include)

# a main that is callable from a console
set(WINDING_NUMBER_MAIN
  src/main.cpp
)

add_executable(winding_number ${WINDING_NUMBER_MAIN})

target_link_libraries(winding_number PRIVATE winding_lib pthread)

# unit tests for the winding number homework problem
set(GTEST ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest)
set(GTEST_SRC_DIR ${GTEST}/src)
set(GTEST_INC_DIR ${GTEST}/include)

set(WINDING_NUMBER_TEST_SRC
  test/winding_test.cpp
  test/poly_io_test.cpp
  test/testmain.cpp
  ${GTEST_SRC_DIR}/gtest-all.cc
)

add_executable(winding_number_test ${WINDING_NUMBER_TEST_SRC})

target_include_directories(winding_number_test PRIVATE ${GTEST_INC_DIR} ${GTEST})
target_link_libraries(winding_number_test PRIVATE winding_lib pthread)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/polygons.txt ${CMAKE_CURRENT_BINARY_DIR}/polygons.txt COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test/crlf_polygons.txt ${CMAKE_CURRENT_BINARY_DIR}/crlf_polygons.txt COPYONLY)

gtest_discover_tests(winding_number_test)

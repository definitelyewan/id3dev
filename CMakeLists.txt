cmake_minimum_required(VERSION 3.10...3.27)

project(id3dev VERSION 26.01)

# Build options
option(BUILD_ID3_SHARED_LIBS "building a shared lib" OFF)
option(BUILD_ID3_DOCS "build docs" OFF)
option(BUILD_ID3_TESTS "build tests" OFF)
option(DEBUG_ID3_SYMBOLS "enables debugging" OFF)
option(BUILD_ID3_C_EXAMPLES "builds examples" OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enable debugging
if(DEBUG_ID3_SYMBOLS)
    add_compile_options("-Wall")
    add_compile_options("-g")
endif()

# Enable shared lib
if(BUILD_ID3_SHARED_LIBS)
    option(BUILD_SHARED_LIBS ON)
endif()

# Build dependencies
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies")

# Build libid3dev
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src")

# Build tests
if(BUILD_ID3_TESTS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()

# Build examples
if(BUILD_ID3_C_EXAMPLES)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/examples")
endif()

# Build docs
if(BUILD_ID3_DOCS)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/docs")
endif()

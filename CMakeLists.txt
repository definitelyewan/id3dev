cmake_minimum_required(VERSION 3.22.1)
project(Id3dev VERSION 1.0.0.0)

option(BUILD_SHARED_LIBS "building a shared lib" ON)
option(BUILD_TESTS "build tests" ON)

set(ID3V1_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1/id3v1.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1/id3v1Parser.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1/id3v1Types.h"
)


set(ID3V1_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v1/id3v1Parser.h"
)

add_library(Id3dev ${ID3V1_HEADERS} ${ID3V1_SRC})
target_include_directories(Id3dev PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1")
set_target_properties(Id3dev PROPERTIES C_STANDARD 99)


add_subdirectory("modules/ByteStream")
add_subdirectory("modules/LinkedListLib")

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()

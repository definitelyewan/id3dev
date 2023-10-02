cmake_minimum_required(VERSION 3.22.1)
project(Id3dev VERSION 1.0.0.0)

option(BUILD_SHARED_LIBS "building a shared lib" ON)
option(BUILD_ID3_TESTS "build tests" ON)
option(DEDUB_SYMBOLS "enables debugging" ON)

set(ID3V1_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1/id3v1.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1/id3v1Parser.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1/id3v1Types.h"
)


set(ID3V1_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v1/id3v1Parser.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v1/id3v1.c"
)

if(DEBUG_SYMBOLS)
  set(CMAKE_C_FLAGS "-g")
  set(CMAKE_C_FLAGS "-Wall")

endif()

add_library(Id3dev ${ID3V1_HEADERS} ${ID3V1_SRC})
target_link_libraries(Id3dev ByteStream)
target_link_libraries(Id3dev m)

target_include_directories(Id3dev PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1")
set_target_properties(Id3dev PROPERTIES C_STANDARD 99)


add_subdirectory("modules/ByteStream")

if(BUILD_ID3_TESTS)
  add_subdirectory(tests)
endif()

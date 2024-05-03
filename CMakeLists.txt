cmake_minimum_required(VERSION 3.22.1)

project(Id3dev VERSION 1.0.0.0)


option(BUILD_SHARED_LIBS "building a shared lib" OFF)
option(BUILD_ID3_DOCS "build docs" OFF)
option(BUILD_ID3_TESTS "build tests" OFF)
option(DEDUB_ID3_SYMBOLS "enables debugging" ON)
option(BUILD_ID3_C_EXAMPLES "builds examples" OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(ID3V1_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1/id3v1.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1/id3v1Parser.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1/id3v1Types.h"
)

set(ID3V2_HEADERS 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v2/id3v2.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v2/id3v2Parser.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v2/id3v2TagIdentity.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v2/id3v2Types.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v2/id3v2Context.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v2/id3v2Frame.h"
)

set(ID3V1_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v1/id3v1Parser.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v1/id3v1.c"
)

set(ID3V2_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v2/id3v2TagIdentity.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v2/id3v2Context.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v2/id3v2Parser.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v2/id3v2Frame.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/id3v2/id3v2.c"
)


#all module files will be redefined here so i can ignore the CMake files they come with

#ByteStream shinanigans
set(BYTE_STREAM_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/include/byteStream.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/include/byteDefines.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/include/byteEndian.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/include/byteInt.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/include/byteStream.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/include/byteTypes.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/include/byteUnicode.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/src/modules/byteEndian.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/src/modules/byteInt.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/src/modules/byteUnicode.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/src/byteStream.c"
)

add_library(ByteStream OBJECT ${BYTE_STREAM_FILES})
target_include_directories(ByteStream PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/ByteStream/include")

#LinkedListLib shinanigans

set(LINKED_LIST_LIB_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/LinkedListLib/include/LinkedList.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/LinkedListLib/src/LinkedList.c"
)

add_library(LinkedListLib OBJECT ${LINKED_LIST_LIB_FILES})
target_include_directories(LinkedListLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/LinkedListLib/include")

#HashTableLib shinanigans

set(HASH_TABLE_LIB_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/HashTableLib/include/HashTable.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/HashTableLib/src/HashTable.c"
)

add_library(HashTableLib OBJECT ${HASH_TABLE_LIB_FILES})
target_include_directories(HashTableLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/HashTableLib/include")



set(ID3DEV_HEADERS "${ID3V1_HEADERS}" "${ID3V2_HEADERS}" "${CMAKE_CURRENT_SOURCE_DIR}/include/id3dev.h")
set(ID3DEV_SRC "${ID3V1_SRC}" "${ID3V2_SRC}" "${CMAKE_CURRENT_SOURCE_DIR}/src/id3dev.c")



if(DEBUG_ID3_SYMBOLS)
  if(MSVC)
    add_compile_options("/W4")
  else()
    add_compile_options("-Wall")
    #add_compile_options("-fsanitize=address")
    add_compile_options("-g")
  endif()
endif()


add_library(Id3dev ${ID3DEV_HEADERS} ${ID3DEV_SRC}
  $<TARGET_OBJECTS:ByteStream>
  $<TARGET_OBJECTS:LinkedListLib>
  $<TARGET_OBJECTS:HashTableLib>
)

target_link_libraries(Id3dev PRIVATE m) #math library

target_include_directories(Id3dev PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(Id3dev PROPERTIES C_STANDARD 99)

if(BUILD_ID3_TESTS)
  #dont build cmocka tests or examples
  option(UNIT_TESTING OFF)
  option(WITH_EXAMPLES OFF)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/id3dependencies/cmocka/")
  add_subdirectory("tests")
endif()

if(BUILD_ID3_DOCS)
  add_subdirectory("docs")
endif()

if(BUILD_ID3_C_EXAMPLES)
  add_subdirectory("examples")
endif()

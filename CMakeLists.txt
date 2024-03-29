cmake_minimum_required(VERSION 3.22.1)

project(Id3dev VERSION 1.0.0.0)


option(BUILD_SHARED_LIBS "building a shared lib" OFF)
option(BUILD_ID3_DOCS "build docs" OFF)
option(BUILD_ID3_TESTS "build tests" OFF)
option(DEDUB_ID3_SYMBOLS "enables debugging" ON)

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
)



set(ID3DEV_HEADERS "${ID3V1_HEADERS}" "${ID3V2_HEADERS}")
set(ID3DEV_SRC "${ID3V1_SRC}" "${ID3V2_SRC}")



if(DEBUG_ID3_SYMBOLS)
  if (MSVC)
    add_compile_options("/W4")
  else()
    add_compile_options("-Wall")
    #add_compile_options("-fsanitize=address")
    add_compile_options("-g")
  endif()
endif()

add_library(Id3dev ${ID3DEV_HEADERS} ${ID3DEV_SRC})
target_link_libraries(Id3dev ByteStream) #byte stream library
target_link_libraries(Id3dev LinkedList) #linked list library
target_link_libraries(Id3dev HashTableLib) #hash table library
target_link_libraries(Id3dev m) #math library

target_include_directories(Id3dev PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v1" "${CMAKE_CURRENT_SOURCE_DIR}/include/id3v2")
set_target_properties(Id3dev PROPERTIES C_STANDARD 99)


add_subdirectory("modules/ByteStream")
add_subdirectory("modules/LinkedListLib")
add_subdirectory("modules/HashTableLib")

if(BUILD_ID3_TESTS)
  #dont build cmocka tests or examples
  option(UNIT_TESTING OFF)
  option(WITH_EXAMPLES OFF)
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/modules/cmocka/")
  add_subdirectory("tests")
endif()

if(BUILD_ID3_DOCS)
  add_subdirectory("docs")
endif()

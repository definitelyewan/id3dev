cmake_minimum_required(VERSION 3.10...3.27)


# Configure a private version of ByteStream
set(BYTE_STREAM_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/src/byteStream.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/src/modules/byteEndian.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/src/modules/byteInt.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/src/modules/byteUnicode.c"
)

set(BYTE_STREAM_HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/include/byteDefines.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/include/byteEndian.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/include/byteInt.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/include/byteStream.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/include/byteTypes.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/include/byteUnicode.h"
)

add_library(ByteStreamInternal OBJECT ${BYTE_STREAM_SOURCE_FILES} ${BYTE_STREAM_HEADER_FILES})

if(BUILD_ID3_TESTS)
    target_include_directories(ByteStreamInternal PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/include")
else()
    target_include_directories(ByteStreamInternal PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ByteStream/include")
endif()

# Configure LinkedListLib
set(LINKED_LIST_LIB_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/LinkedListLib/src/LinkedList.c"
)

set(LINKED_LIST_LIB_HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/LinkedListLib/include/LinkedList.h"
)

add_library(LinkedListLib OBJECT ${LINKED_LIST_LIB_SOURCE_FILES} ${LINKED_LIST_LIB_HEADER_FILES})
target_include_directories(LinkedListLib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/LinkedListLib/include")

# Configure HashTableLib

set(HASH_TABLE_LIB_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/HashTableLib/src/hashTable.c"
)

set(HASH_TABLE_LIB_HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/HashTableLib/include/hashTable.h"
)
add_library(HashTableLib OBJECT ${HASH_TABLE_LIB_SOURCE_FILES} ${HASH_TABLE_LIB_HEADER_FILES})
target_include_directories(HashTableLib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/HashTableLib/include")

# Configure cmocka
if(BUILD_ID3_TESTS)
    option(UNIT_TESTING OFF)
    option(WITH_EXAMPLES OFF)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/cmocka")
endif()


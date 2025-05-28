cmake_minimum_required(VERSION 3.22.1)

# Enable shared lib
if(BUILD_ID3_SHARED_LIBS)
    option(BUILD_SHARED_LIBS ON)
endif()

# Configure libid3dev

set(ID3V1_HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v1/id3v1.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v1/id3v1Parser.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v1/id3v1Types.h"
)

set(ID3V1_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/id3v1/id3v1Parser.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/id3v1/id3v1.c"
)

set(ID3V2_HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2Parser.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2TagIdentity.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2Types.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2Context.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2Frame.h"
)

set(ID3V2_SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/id3v2/id3v2TagIdentity.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/id3v2/id3v2Context.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/id3v2/id3v2Parser.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/id3v2/id3v2Frame.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/id3v2/id3v2.c"
)

set(ID3DEV_HEADER_FILES
        ${ID3V1_HEADERS}
        ${ID3V2_HEADERS}
        "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3dev.h"
)

set(ID3DEV_SOURCE_FILES
        ${ID3V1_SOURCE_FILES}
        ${ID3V2_SOURCE_FILES}
        "${CMAKE_CURRENT_SOURCE_DIR}/id3dev.c"
)

add_library(id3dev ${ID3DEV_HEADER_FILES} ${ID3DEV_SOURCE_FILES}
        $<TARGET_OBJECTS:LinkedListLib>
        $<TARGET_OBJECTS:HashTableLib>
)

target_link_libraries(id3dev PRIVATE ByteStreamInternal)
target_link_libraries(id3dev PRIVATE m)
target_include_directories(id3dev PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../"
)

set_target_properties(id3dev PROPERTIES C_STANDARD 99)

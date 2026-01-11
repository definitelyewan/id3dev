cmake_minimum_required(VERSION 3.22.1)

find_package(Doxygen)

if(DOXYGEN_FOUND)


    set(ID3DEV_DOC_FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox"

            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3dev.h"

            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v1/id3v1.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v1/id3v1Parser.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v1/id3v1Types.h"

            "${CMAKE_CURRENT_SOURCE_DIR}/../src/id3v1/id3v1.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/id3v1/id3v1Parser.c"

            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2Context.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2Frame.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2Parser.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2TagIdentity.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3v2/id3v2Types.h"

            "${CMAKE_CURRENT_SOURCE_DIR}/../src/id3v2/id3v2.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/id3v2/id3v2Context.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/id3v2/id3v2Frame.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/id3v2/id3v2Parser.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/id3v2/id3v2TagIdentity.c"
    )

    set(DOXYGEN_PROJECT_NAME ${PROJECT_NAME})
    set(DOXYGEN_PROJECT_NUMBER ${PROJECT_VERSION})
    set(DOXYGEN_PROJECT_BRIEF "An ID3 metadata library")

    set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
    set(DOXYGEN_MARKDOWN_SUPPORT YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_PRIVATE YES)
    set(DOXYGEN_EXTRACT_STATIC YES)
    set(DOXYGEN_ALWAYS_DETAILED_SEC YES)
    set(DOXYGEN_REPEAT_BRIEF YES)
    set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
    set(DOXYGEN_SOURCE_BROWSER YES)
    set(DOXYGEN_INLINE_SOURCES NO)
    set(DOXYGEN_EXCLUDE_SYMBOLS "internal_*")

    set(DOXYGEN_LAYOUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/DoxygenLayout.xml)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doxygen)

    doxygen_add_docs(docs
                    ${ID3DEV_DOC_FILES}
                    COMMENT "Generating API documentation with Doxygen")


endif()
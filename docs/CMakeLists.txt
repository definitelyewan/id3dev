cmake_minimum_required(VERSION 3.22.1)

find_package(Doxygen)

if(DOXYGEN_FOUND)


    set(ID3DEV_DOC_FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox"

            "${CMAKE_CURRENT_SOURCE_DIR}/../include/id3dev.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../src/id3dev.c"

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
    # Name the html folders after their published locations (docs/doxygen, docs/python)
    # so the relative "C API" / "Python API" links work in the build tree too.
    set(DOXYGEN_HTML_OUTPUT doxygen)

    doxygen_add_docs(docs
                    ${ID3DEV_DOC_FILES}
                    COMMENT "Generating API documentation with Doxygen")

    # Python bindings: a second, separate site with its own navigation tree.
    # doxygen_add_docs reads the DOXYGEN_* variables when it is called, so
    # changing them here only affects this target.
    # Output: <build>/docs/doxygen/python, next to the C site's <build>/docs/doxygen/doxygen.
    set(ID3DEV_PYTHON_DOC_FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/python.dox"
            "${CMAKE_CURRENT_SOURCE_DIR}/../interfaces/python/id3dev/src/id3dev"
    )

    set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME} for Python")
    set(DOXYGEN_PROJECT_BRIEF "Python bindings for id3dev")
    set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C NO)
    set(DOXYGEN_OPTIMIZE_OUTPUT_JAVA YES)   # doxygen's recommended mode for Python
    set(DOXYGEN_EXTRACT_ALL NO)             # only document what has a docstring...
    set(DOXYGEN_HIDE_UNDOC_MEMBERS YES)     # ...which keeps _private helpers out
    set(DOXYGEN_EXTRACT_PRIVATE NO)
    set(DOXYGEN_EXTRACT_STATIC NO)
    set(DOXYGEN_SOURCE_BROWSER NO)
    set(DOXYGEN_FILE_PATTERNS *.py *.dox)
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_EXCLUDE_PATTERNS */__pycache__/* */_util.py)
    set(DOXYGEN_LAYOUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/DoxygenLayoutPython.xml)
    set(DOXYGEN_HTML_OUTPUT python)

    doxygen_add_docs(docs_python
                    ${ID3DEV_PYTHON_DOC_FILES}
                    COMMENT "Generating Python API documentation with Doxygen")


endif()
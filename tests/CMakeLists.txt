cmake_minimum_required(VERSION 3.10...3.27)


set(TEST_ASSETS
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/Beetlebum.mp3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/boniver.mp3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/dannybrown.mp3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/danybrown2.mp3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/empty.mp3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/OnGP.mp3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/sorry4dying.mp3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/null.mp3"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/cat.png"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/danybrown2.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/sorry4dying.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets/beetlebum.json"
)


set(TEST_ID3V1 "${CMAKE_CURRENT_SOURCE_DIR}/id3v1Functions.c")
set(TEST_ID3V2_TAG_IDENTITY_ID3V2 "${CMAKE_CURRENT_SOURCE_DIR}/id3v2TagIdentityFunctions.c")
set(TEST_ID3V2_CONTEXT "${CMAKE_CURRENT_SOURCE_DIR}/id3v2ContextFunctions.c")
set(TEST_ID3V2_PARSER "${CMAKE_CURRENT_SOURCE_DIR}/id3v2ParserFunctions.c")
set(TEST_ID3V2_FRAME "${CMAKE_CURRENT_SOURCE_DIR}/id3v2FrameFunctions.c")
set(TEST_ID3V2 "${CMAKE_CURRENT_SOURCE_DIR}/id3v2Functions.c")
set(TEST_ID3DEV "${CMAKE_CURRENT_SOURCE_DIR}/id3devFunctions.c")

file(
        COPY ${TEST_ASSETS}
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets"
)

# id3v1 tests
add_executable(id3v1_test ${TEST_ID3V1})
set_target_properties(id3v1_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v1_test PRIVATE id3dev)
target_link_libraries(id3v1_test PRIVATE ByteStreamInternal)
target_link_libraries(id3v1_test PRIVATE cmocka)

# id3v2 tests
add_executable(id3v2_tag_identity_test ${TEST_ID3V2_TAG_IDENTITY_ID3V2})
set_target_properties(id3v2_tag_identity_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v2_tag_identity_test PRIVATE id3dev)
target_link_libraries(id3v2_tag_identity_test PRIVATE ByteStreamInternal)
target_link_libraries(id3v2_tag_identity_test PRIVATE cmocka)

add_executable(id3v2_context_test ${TEST_ID3V2_CONTEXT})
set_target_properties(id3v2_context_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v2_context_test PRIVATE id3dev)
target_link_libraries(id3v2_context_test PRIVATE ByteStreamInternal)
target_link_libraries(id3v2_context_test PRIVATE cmocka)

add_executable(id3v2_parser_test ${TEST_ID3V2_PARSER})
set_target_properties(id3v2_parser_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v2_parser_test PRIVATE id3dev)
target_link_libraries(id3v2_parser_test PRIVATE ByteStreamInternal)
target_link_libraries(id3v2_parser_test PRIVATE cmocka)

add_executable(id3v2_frame_test ${TEST_ID3V2_FRAME})
set_target_properties(id3v2_frame_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v2_frame_test PRIVATE id3dev)
target_link_libraries(id3v2_frame_test PRIVATE ByteStreamInternal)
target_link_libraries(id3v2_frame_test PRIVATE cmocka)

add_executable(id3v2_test ${TEST_ID3V2})
set_target_properties(id3v2_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v2_test PRIVATE id3dev)
target_link_libraries(id3v2_test PRIVATE ByteStreamInternal)
target_link_libraries(id3v2_test PRIVATE cmocka)

add_executable(id3dev_test ${TEST_ID3DEV})
set_target_properties(id3dev_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3dev_test PRIVATE id3dev)
target_link_libraries(id3dev_test PRIVATE ByteStreamInternal)
target_link_libraries(id3dev_test PRIVATE cmocka)

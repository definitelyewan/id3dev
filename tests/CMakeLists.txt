cmake_minimum_required(VERSION 3.22.1)

set(TEST_ID3V1 "${CMAKE_CURRENT_SOURCE_DIR}/id3v1Functions.c")
set(TEST_ID3V2_TAG_IDENTITY_ID3V2 "${CMAKE_CURRENT_SOURCE_DIR}/id3v2TagIdentityFunctions.c")
set(TEST_ID3V2_CONTEXT "${CMAKE_CURRENT_SOURCE_DIR}/id3v2ContextFunctions.c")
set(TEST_ID3V2_PARSER "${CMAKE_CURRENT_SOURCE_DIR}/id3v2ParserFunctions.c")

set(TEST_ASSETS
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/Archangel.mp3"
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/Beetlebum.mp3"
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/boniver.mp3"
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/dannybrown.mp3"
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/danybrown2.mp3"
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/empty.mp3"
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/emptyb.mp3"
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/OnGP.mp3"
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/sorry4dying.mp3"
  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/null.mp3"
)


#id3v1 tests
add_executable(id3v1_test ${TEST_ID3V1})
set_target_properties(id3v1_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v1_test PRIVATE Id3dev)
target_link_libraries(id3v1_test PRIVATE ByteStream)
target_link_libraries(id3v1_test PRIVATE cmocka)

#id3v2 tests
add_executable(id3v2_tag_identity_test ${TEST_ID3V2_TAG_IDENTITY_ID3V2})
set_target_properties(id3v2_tag_identity_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v2_tag_identity_test PRIVATE Id3dev)
target_link_libraries(id3v2_tag_identity_test PRIVATE cmocka)

add_executable(id3v2_context_test ${TEST_ID3V2_CONTEXT})
set_target_properties(id3v2_context_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v2_context_test PRIVATE Id3dev)
target_link_libraries(id3v2_context_test PRIVATE cmocka)

add_executable(id3v2_parser_test ${TEST_ID3V2_PARSER})
set_target_properties(id3v2_parser_test PROPERTIES C_STANDARD 99)
target_link_libraries(id3v2_parser_test PRIVATE Id3dev)
target_link_libraries(id3v2_parser_test PRIVATE ByteStream)
target_link_libraries(id3v2_parser_test PRIVATE cmocka)


file(
  COPY ${TEST_ASSETS}
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets"
)
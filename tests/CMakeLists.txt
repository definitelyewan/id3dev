cmake_minimum_required(VERSION 3.22.1)

find_library(CMOCKA_LIBRARIES REQUIRED NAMES cmocka)
if(CMOCKA_LIBRARIES)
    message(STATUS "cmocka library found: ${CMOCKA_LIBRARIES}")
else()
    #cmocka not found
    message(FATAL_ERROR "cmocka library not found please install cmocka to run these tests")
endif()

set(TEST_ID3V1 "${CMAKE_CURRENT_SOURCE_DIR}/id3v1Functions.c")

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

add_executable(id3v1_test ${TEST_ID3V1})


set_target_properties(id3v1_test PROPERTIES C_STANDARD 99)

target_link_libraries(id3v1_test PRIVATE Id3dev)
target_link_libraries(id3v1_test PRIVATE ByteStream)
target_link_libraries(id3v1_test PRIVATE cmocka)



file(
  COPY ${TEST_ASSETS}
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets"
)
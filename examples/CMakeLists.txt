cmake_minimum_required(VERSION 3.10...3.27)

#All c examples

add_executable(id3dev_printInfo_c_example "${CMAKE_CURRENT_SOURCE_DIR}/c/printInfo.c")
set_target_properties(id3dev_printInfo_c_example PROPERTIES C_STANDARD 99)
target_link_libraries(id3dev_printInfo_c_example PRIVATE id3dev)

add_executable(id3dev_savePicture_c_example "${CMAKE_CURRENT_SOURCE_DIR}/c/savePicture.c")
set_target_properties(id3dev_savePicture_c_example PROPERTIES C_STANDARD 99)
target_link_libraries(id3dev_savePicture_c_example PRIVATE id3dev)

add_executable(id3dev_listAllFrames_c_example "${CMAKE_CURRENT_SOURCE_DIR}/c/listAllFrames.c")
set_target_properties(id3dev_listAllFrames_c_example PROPERTIES C_STANDARD 99)
target_link_libraries(id3dev_listAllFrames_c_example PRIVATE id3dev)

add_executable(id3dev_displayAllText_c_example "${CMAKE_CURRENT_SOURCE_DIR}/c/displayAllText.c")
set_target_properties(id3dev_displayAllText_c_example PROPERTIES C_STANDARD 99)
target_link_libraries(id3dev_displayAllText_c_example PRIVATE id3dev)

add_executable(id3dev_buildTag_c_example "${CMAKE_CURRENT_SOURCE_DIR}/c/buildTag.c")
set_target_properties(id3dev_buildTag_c_example PROPERTIES C_STANDARD 99)
target_link_libraries(id3dev_buildTag_c_example PRIVATE id3dev)

add_executable(id3dev_extractTag_c_example "${CMAKE_CURRENT_SOURCE_DIR}/c/extractTag.c")
set_target_properties(id3dev_extractTag_c_example PROPERTIES C_STANDARD 99)
target_link_libraries(id3dev_extractTag_c_example PRIVATE id3dev)
find_path(PYTHON_HEADER_DIR Python.h)
if(NOT PYTHON_HEADER_DIR)
    message(FATAL_ERROR "Python.h not found")
endif()
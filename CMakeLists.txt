CMAKE_MINIMUM_REQUIRED(VERSION 3.4.0)

project(meta_log)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(meta_log
    src/logger.cpp
    src/file_writer.cpp
    src/tool.cpp
)
target_include_directories(meta_log PRIVATE include)

add_executable(meta_log_test
    src/logger.cpp
    src/file_writer.cpp
    src/tool.cpp
    test/log_test.cpp
)
target_include_directories(meta_log_test PRIVATE include)

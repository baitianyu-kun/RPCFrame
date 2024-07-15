cmake_minimum_required(VERSION 3.10)

project(RPCFrame VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 可执行文件的存放路径
set(EXECUTABLE_OUTPUT_PATH ./bin)
# 头文件搜索目录
set(RPCFrame_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/rocket/include)

include_directories(${RPCFrame_SRC_INCLUDE_DIR})
add_executable(log_test ./testcases/log_test.cpp
        rocket/include/net/eventloop.h
        rocket/net/eventloop.cpp
        rocket/include/common/config.h
        rocket/common/config.cpp
        rocket/include/common/log.h
        rocket/common/log.cpp
        rocket/include/common/mutex.h
        rocket/common/util.cpp
        rocket/include/common/util.h
)

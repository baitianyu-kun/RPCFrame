cmake_minimum_required(VERSION 3.10)

project(RPCFrame VERSION 1.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug") # 设置为debug即-g编译选项

link_libraries("/usr/lib/libprotobuf.a")
link_libraries("/usr/lib/libtinyxml.a") # 添加第三方库

include_directories(${PROJECT_SOURCE_DIR}/rocket/include) # 头文件搜索

aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/common SRC_COMMON_LIST) # 读取所有源文件并放到SRC_LIST中，但是不会对子目录进行递归调用
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net SRC_NET_LIST)

add_executable(test_eventloop ${SRC_COMMON_LIST} ${SRC_NET_LIST} testcases/test_eventloop.cpp)

target_sources(test_eventloop PUBLIC ${SRC_COMMON_LIST} ${SRC_NET_LIST})
target_link_libraries(test_eventloop pthread)




# 可执行文件的存放路径
#set(EXECUTABLE_OUTPUT_PATH ./bin)
## 头文件搜索目录
#set(RPCFrame_SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/rocket/include)
#
#include_directories(${RPCFrame_SRC_INCLUDE_DIR})
#add_executable(log_test ./testcases/log_test.cpp
#        rocket/include/net/eventloop.h
#        rocket/net/eventloop.cpp
#        rocket/include/common/config.h
#        rocket/common/config.cpp
#        rocket/include/common/log.h
#        rocket/common/log.cpp
#        rocket/include/common/mutex.h
#        rocket/common/util.cpp
#        rocket/include/common/util.h
#        rocket/include/net/fd_event.h
#        rocket/net/fd_event.cpp
#        rocket/include/net/timer_event.h
#        rocket/net/timer_event.cpp
#)

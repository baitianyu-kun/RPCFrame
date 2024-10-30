cmake_minimum_required(VERSION 3.10)

project(RPCFrame VERSION 1.0)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug") # 设置为debug即-g编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed -ldl")
enable_language(ASM)

# 设置文件输出路径
set(PATH_LIB ${PROJECT_SOURCE_DIR}/lib)
set(PATH_BIN ${PROJECT_SOURCE_DIR}/bin)
set(PATH_TESTCASES testcases)

# 头文件搜索
include_directories(${PROJECT_SOURCE_DIR}/rocket/include)

# 读取所有源文件并放到SRC_LIST中，但是不会对子目录进行递归调用
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/common SRC_COMMON_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net SRC_NET_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/tcp SRC_NET_TCP_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/coder/tinypb SRC_NET_CODER_TINYPB_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/coder/http SRC_NET_CODER_HTTP_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/rpc SRC_NET_RPC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/register SRC_NET_REGISTER_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/balance SRC_NET_BALANCE_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/rpc/dispatcher SRC_NET_RPC_DISPATCHER_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/coroutine SRC_COROUTINE_LIST)

# 本项目的lib
set(COXTX ${PROJECT_SOURCE_DIR}/rocket/include/coroutine/coctx_swap.S)
add_library(rocket ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST}
        ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} ${SRC_NET_REGISTER_LIST} ${SRC_NET_BALANCE_LIST}
        ${SRC_NET_RPC_DISPATCHER_LIST} ${SRC_COROUTINE_LIST} ${COXTX})
install(TARGETS rocket DESTINATION ${PATH_LIB})

# 第三方lib
set(PROTOBUF_LIB /usr/lib/libprotobuf.a)
set(TINYXML_LIB /usr/lib/libtinyxml.a)

# 总lib
set(ALL_LIBS rocket ${PROTOBUF_LIB} ${TINYXML_LIB} pthread)

# test rpc server
add_executable(test_rpc_server
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_rpc_server.cpp
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/order.pb.cc
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/order.pb.h
        rocket/include/coroutine/coroutine_pool.h)
target_link_libraries(test_rpc_server ${ALL_LIBS})
install(TARGETS test_rpc_server DESTINATION ${PATH_BIN})

# test rpc client
add_executable(test_rpc_client
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_rpc_client.cpp
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/order.pb.cc
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/order.pb.h
        rocket/include/coroutine/coroutine_pool.h)
target_link_libraries(test_rpc_client ${ALL_LIBS})
install(TARGETS test_rpc_client DESTINATION ${PATH_BIN})

# test rpc register
add_executable(test_rpc_register
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_rpc_register.cpp
        rocket/include/coroutine/coroutine_pool.h)
target_link_libraries(test_rpc_register ${ALL_LIBS})
install(TARGETS test_rpc_register DESTINATION ${PATH_BIN})

# test routine
add_executable(test_coroutine
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_coroutine.cpp)
target_link_libraries(test_coroutine ${ALL_LIBS})
install(TARGETS test_coroutine DESTINATION ${PATH_BIN})

# test routine 2
add_executable(test_coroutine2
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_coroutine2.cpp)
target_link_libraries(test_coroutine2 ${ALL_LIBS})
install(TARGETS test_coroutine2 DESTINATION ${PATH_BIN})

# test routine 3
add_executable(test_coroutine3
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_coroutine3.cpp)
target_link_libraries(test_coroutine3 ${ALL_LIBS})
install(TARGETS test_coroutine3 DESTINATION ${PATH_BIN})


# test routine 4
add_executable(test_coroutine4
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_coroutine4.cpp)
target_link_libraries(test_coroutine4 ${ALL_LIBS})
install(TARGETS test_coroutine4 DESTINATION ${PATH_BIN})

# test routine connect
add_executable(test_coroutine_connect
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_coroutine_connect.cpp
        rocket/include/coroutine/coroutine_pool.h)
target_link_libraries(test_coroutine_connect ${ALL_LIBS})
install(TARGETS test_coroutine_connect DESTINATION ${PATH_BIN})


# test con hash
add_executable(test_con_hash
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_con_hash.cpp)
target_link_libraries(test_con_hash ${ALL_LIBS})
install(TARGETS test_con_hash DESTINATION ${PATH_BIN})
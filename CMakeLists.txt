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
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/tcp SRC_NET_TCP_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/coder/tinypb SRC_NET_CODER_TINYPB_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/coder/http SRC_NET_CODER_HTTP_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/rpc SRC_NET_RPC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/rpc/dispatcher SRC_NET_RPC_DISPATCHER_LIST)

# test event loop
add_executable(test_eventloop ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} testcases/test_eventloop.cpp)
target_sources(test_eventloop PUBLIC ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST})
target_link_libraries(test_eventloop pthread)

# test tcp
add_executable(test_tcp ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} testcases/test_tcp.cpp
        rocket/include/net/coder/abstract_coder.h)
target_sources(test_tcp PUBLIC ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST})
target_link_libraries(test_tcp pthread)

# test client
add_executable(test_client ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} testcases/test_client.cpp
        rocket/include/net/rpc/rpc_controller.h
        rocket/include/net/rpc/rpc_closure.h)
target_sources(test_client PUBLIC ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST})
target_link_libraries(test_client pthread)

# test rpc client
add_executable(test_rpc_client ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} ${SRC_NET_RPC_DISPATCHER_LIST} testcases/test_rpc_client.cpp
        testcases/order.pb.cc
        testcases/order.pb.h
        rocket/include/net/register/register_center.h
        rocket/src/net/register/register_center.cpp
        rocket/include/common/encode_util.h
        rocket/src/common/encode_util.cpp
        rocket/include/net/rpc/dispatcher/register_dispatcher.h
        rocket/src/net/rpc/dispatcher/register_dispatcher.cpp)
target_sources(test_rpc_client PUBLIC ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} ${SRC_NET_RPC_DISPATCHER_LIST})
target_link_libraries(test_rpc_client pthread)

# test rpc server
add_executable(test_rpc_server ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} ${SRC_NET_RPC_DISPATCHER_LIST} testcases/test_rpc_server.cpp
        testcases/order.pb.cc
        testcases/order.pb.h
        rocket/include/net/register/register_center.h
        rocket/src/net/register/register_center.cpp
        rocket/include/common/encode_util.h
        rocket/src/common/encode_util.cpp
        rocket/include/net/rpc/dispatcher/register_dispatcher.h
        rocket/src/net/rpc/dispatcher/register_dispatcher.cpp)
target_sources(test_rpc_server PUBLIC ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} ${SRC_NET_RPC_DISPATCHER_LIST})
target_link_libraries(test_rpc_server pthread)

# test rpc register
add_executable(test_rpc_register ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} ${SRC_NET_RPC_DISPATCHER_LIST} testcases/test_rpc_register.cpp
        rocket/include/net/register/register_center.h
        rocket/src/net/register/register_center.cpp
        rocket/include/common/encode_util.h
        rocket/src/common/encode_util.cpp
        rocket/include/net/rpc/dispatcher/register_dispatcher.h
        rocket/src/net/rpc/dispatcher/register_dispatcher.cpp)
target_sources(test_rpc_register PUBLIC ${SRC_COMMON_LIST} ${SRC_NET_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_CODER_TINYPB_LIST} ${SRC_NET_CODER_HTTP_LIST} ${SRC_NET_RPC_LIST} ${SRC_NET_RPC_DISPATCHER_LIST})
target_link_libraries(test_rpc_register pthread)
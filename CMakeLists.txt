cmake_minimum_required(VERSION 3.10)

project(RPCFrame VERSION 1.0)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug") # 设置为debug即-g编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed -ldl")

# 设置文件输出路径
set(PATH_LIB ${PROJECT_SOURCE_DIR}/lib)
set(PATH_BIN ${PROJECT_SOURCE_DIR}/bin)
set(PATH_TESTCASES testcases)

# 头文件搜索
include_directories(${PROJECT_SOURCE_DIR}/rocket/include)

# 读取所有源文件并放到SRC_LIST中，但是不会对子目录进行递归调用
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/common SRC_COMMON_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/event SRC_EVENT_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/rpc SRC_RPC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/rpc/servlet SRC_RPC_SERVLET_LIST)

aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net SRC_NET_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/protocol SRC_NET_PROTOCOL_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/protocol/http SRC_NET_HTTP_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/protocol/tinypb SRC_NET_TINYPB_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/tcp SRC_NET_TCP_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/socket SRC_NET_SOCKET_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/rocket/src/net/balance SRC_NET_BALANCE_LIST)

# 本项目的lib
add_library(rocket ${SRC_COMMON_LIST} ${SRC_EVENT_LIST} ${SRC_RPC_LIST} ${SRC_RPC_SERVLET_LIST} ${SRC_NET_LIST} ${SRC_NET_PROTOCOL_LIST}
        ${SRC_NET_HTTP_LIST} ${SRC_NET_TINYPB_LIST} ${SRC_NET_TCP_LIST} ${SRC_NET_SOCKET_LIST} ${SRC_NET_BALANCE_LIST})
install(TARGETS rocket DESTINATION ${PATH_LIB})

# 第三方lib
set(PROTOBUF_LIB /usr/lib/libprotobuf.a)
set(TINYXML_LIB /usr/lib/libtinyxml.a)

# 总lib
set(ALL_LIBS rocket ${PROTOBUF_LIB} ${TINYXML_LIB} pthread)

# test rpc server
add_executable(test_eventloop
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_eventloop.cpp
        rocket/include/net/tcp/abstract_tcp_buffer.h
        rocket/include/net/protocol/abstract_protocol.h
        rocket/include/net/protocol/http/http_define.h
        rocket/include/net/protocol/http/http_parse.h
        rocket/include/net/protocol/http/http_servlet.h
        rocket/include/rpc/rpc_dispatcher.h
        rocket/include/rpc/register_center.h
        rocket/include/rpc/servlet/register_center_servlet.h
        rocket/src/rpc/servlet/register_center_servlet.cpp
        rocket/include/rpc/servlet/server_servlet.h
        rocket/src/rpc/servlet/server_servlet.cpp
        rocket/include/net/tcp/tcp_connection.h
        rocket/include/net/tcp/tcp_server.h
        rocket/include/net/tcp/tcp_client.h
        rocket/include/rpc/rpc_server.h
        rocket/include/rpc/rpc_controller.h
        rocket/include/rpc/rpc_closure.h
        rocket/include/rpc/rpc_channel.h
        rocket/include/net/balance/hash_balance.h
)
target_link_libraries(test_eventloop ${ALL_LIBS})

add_executable(test_http_parse_encode
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_http_parse_encode.cpp
        rocket/include/net/tcp/abstract_tcp_buffer.h
        rocket/include/net/protocol/abstract_protocol.h
        rocket/include/net/protocol/http/http_define.h
        rocket/include/net/protocol/http/http_parse.h
        rocket/include/rpc/rpc_dispatcher.h
        rocket/include/rpc/register_center.h
        rocket/include/rpc/servlet/register_center_servlet.h
        rocket/src/rpc/servlet/register_center_servlet.cpp
        rocket/include/rpc/servlet/server_servlet.h
        rocket/src/rpc/servlet/server_servlet.cpp
        rocket/include/net/tcp/tcp_connection.h
        rocket/include/net/tcp/tcp_server.h
        rocket/include/net/tcp/tcp_client.h
        rocket/include/rpc/rpc_server.h
        rocket/include/rpc/rpc_controller.h
        rocket/include/rpc/rpc_closure.h
        rocket/include/rpc/rpc_channel.h
        rocket/include/net/balance/hash_balance.h
)
target_link_libraries(test_http_parse_encode ${ALL_LIBS})

add_executable(test_http_servlet
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_http_servlet.cpp
        rocket/include/net/tcp/abstract_tcp_buffer.h
        rocket/include/net/protocol/abstract_protocol.h
        rocket/include/net/protocol/http/http_define.h
        rocket/include/net/protocol/http/http_parse.h
        rocket/include/rpc/rpc_dispatcher.h
        rocket/include/rpc/register_center.h
        rocket/include/rpc/servlet/register_center_servlet.h
        rocket/src/rpc/servlet/register_center_servlet.cpp
        rocket/include/rpc/servlet/server_servlet.h
        rocket/src/rpc/servlet/server_servlet.cpp
        rocket/include/net/tcp/tcp_connection.h
        rocket/include/net/tcp/tcp_server.h
        rocket/include/net/tcp/tcp_client.h
        rocket/include/rpc/rpc_server.h
        rocket/include/rpc/rpc_controller.h
        rocket/include/rpc/rpc_closure.h
        rocket/include/rpc/rpc_channel.h
        rocket/include/net/balance/hash_balance.h
)
target_link_libraries(test_http_servlet ${ALL_LIBS})

add_executable(test_rpc_dispatcher
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_rpc_dispatcher.cpp
        rocket/include/rpc/register_center.h
        rocket/include/rpc/servlet/register_center_servlet.h
        rocket/src/rpc/servlet/register_center_servlet.cpp
        rocket/include/rpc/servlet/server_servlet.h
        rocket/src/rpc/servlet/server_servlet.cpp
        rocket/include/net/tcp/tcp_connection.h
        rocket/include/net/tcp/tcp_server.h
        rocket/include/net/tcp/tcp_client.h
        rocket/include/rpc/rpc_server.h
        rocket/include/rpc/rpc_controller.h
        rocket/include/rpc/rpc_closure.h
        rocket/include/rpc/rpc_channel.h
        rocket/include/net/balance/hash_balance.h
)
target_link_libraries(test_rpc_dispatcher ${ALL_LIBS})

add_executable(test_tcp_server
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_tcp_server.cpp
        rocket/include/rpc/register_center.h
        rocket/include/rpc/servlet/register_center_servlet.h
        rocket/src/rpc/servlet/register_center_servlet.cpp
        rocket/include/rpc/servlet/server_servlet.h
        rocket/src/rpc/servlet/server_servlet.cpp
        rocket/include/net/tcp/tcp_connection.h
        rocket/include/net/tcp/tcp_server.h
        rocket/include/net/tcp/tcp_client.h
        rocket/include/rpc/rpc_server.h
        rocket/include/rpc/rpc_controller.h
        rocket/include/rpc/rpc_closure.h
        rocket/include/rpc/rpc_channel.h
        rocket/include/net/balance/hash_balance.h
)
target_link_libraries(test_tcp_server ${ALL_LIBS})

add_executable(test_tcp_client
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_tcp_client.cpp
        rocket/include/rpc/register_center.h
        rocket/include/rpc/servlet/register_center_servlet.h
        rocket/src/rpc/servlet/register_center_servlet.cpp
        rocket/include/rpc/servlet/server_servlet.h
        rocket/src/rpc/servlet/server_servlet.cpp
        rocket/include/net/tcp/tcp_connection.h
        rocket/include/net/tcp/tcp_server.h
        rocket/include/net/tcp/tcp_client.h
        rocket/include/rpc/rpc_server.h
        rocket/include/rpc/rpc_controller.h
        rocket/include/rpc/rpc_closure.h
        rocket/include/rpc/rpc_channel.h
        rocket/include/net/balance/hash_balance.h
)
target_link_libraries(test_tcp_client ${ALL_LIBS})

add_executable(test_rpc_server
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_rpc_server.cpp
        rocket/include/rpc/register_center.h
        rocket/include/rpc/servlet/register_center_servlet.h
        rocket/src/rpc/servlet/register_center_servlet.cpp
        rocket/include/rpc/servlet/server_servlet.h
        rocket/src/rpc/servlet/server_servlet.cpp
        rocket/include/net/tcp/tcp_connection.h
        rocket/include/net/tcp/tcp_server.h
        rocket/include/net/tcp/tcp_client.h
        rocket/include/rpc/rpc_server.h
        rocket/include/rpc/rpc_controller.h
        rocket/include/rpc/rpc_closure.h
        rocket/include/rpc/rpc_channel.h
        rocket/include/net/balance/hash_balance.h
        testcases/order.pb.cc
        testcases/order.pb.h
)
target_link_libraries(test_rpc_server ${ALL_LIBS})

add_executable(test_rpc_channel
        ${PROJECT_SOURCE_DIR}/${PATH_TESTCASES}/test_rpc_channel.cpp
        rocket/include/rpc/register_center.h
        rocket/include/rpc/servlet/register_center_servlet.h
        rocket/src/rpc/servlet/register_center_servlet.cpp
        rocket/include/rpc/servlet/server_servlet.h
        rocket/src/rpc/servlet/server_servlet.cpp
        rocket/include/net/tcp/tcp_connection.h
        rocket/include/net/tcp/tcp_server.h
        rocket/include/net/tcp/tcp_client.h
        rocket/include/rpc/rpc_server.h
        rocket/include/rpc/rpc_controller.h
        rocket/include/rpc/rpc_closure.h
        rocket/include/rpc/rpc_channel.h
        rocket/include/net/balance/hash_balance.h
        testcases/order.pb.cc
        testcases/order.pb.h
)
target_link_libraries(test_rpc_channel ${ALL_LIBS})
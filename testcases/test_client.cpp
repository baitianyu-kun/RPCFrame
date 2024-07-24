//
// Created by baitianyu on 7/21/24.
//
#include <assert.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <unistd.h>
#include "common/log.h"
#include "common/config.h"
#include "common/log.h"
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_client.h"
#include "net/coder/string_coder.h"
#include "net/coder/abstract_protocol.h"
#include "net/coder/tinypb_protocol.h"
#include "net/coder/tinypb_coder.h"

void test_connect_client_tinypb() {
    rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
    rocket::TCPClient client(addr);
    client.connect([addr, &client]() {
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        auto message = std::make_shared<rocket::TinyPBProtocol>();
        message->m_msg_id = "123456789";
        message->m_pb_data = "test pb data";
        message->m_err_info="no error";
        message->m_method_name="call back method";
        message->m_err_code = 123;
        client.writeMessage(message, [](rocket::AbstractProtocol::abstract_pro_sptr_t_ msg_ptr) {
            DEBUGLOG("send message success");
        });
        client.readMessage("123456789", [](rocket::AbstractProtocol::abstract_pro_sptr_t_ msg_ptr) {
            std::shared_ptr<rocket::TinyPBProtocol> message = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(
                    msg_ptr);
            DEBUGLOG("msg_id[%s], get response %s", message->m_msg_id.c_str(), message->m_pb_data.c_str());
        });
    });
}

void test_connect_client_and_write_and_read() {
    rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
    rocket::TCPClient client(addr);
    client.connect([addr, &client]() {
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        auto message = std::make_shared<rocket::StringProtocol>();
        message->m_msg_id = "123456";
        message->info = "hello rocket ";
        client.writeMessage(message, [](rocket::AbstractProtocol::abstract_pro_sptr_t_ msg_ptr) {
            DEBUGLOG("send message success");
        });

        client.readMessage("123456", [](rocket::AbstractProtocol::abstract_pro_sptr_t_ msg_ptr) {
            auto message = std::dynamic_pointer_cast<rocket::StringProtocol>(msg_ptr);
            DEBUGLOG("msg_id[%s], get response %s", message->m_msg_id.c_str(), message->info.c_str());
        });

        client.writeMessage(message, [](rocket::AbstractProtocol::abstract_pro_sptr_t_ msg_ptr) {
            DEBUGLOG("send message222 success");
        });
    });
}

void test_connect_client_and_write() {
    rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
    rocket::TCPClient client(addr);
    client.connect([addr, &client]() {
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        auto message = std::make_shared<rocket::StringProtocol>();
        message->m_msg_id = "1231";
        message->info = "hello rocket";
        client.writeMessage(message, [](rocket::AbstractProtocol::abstract_pro_sptr_t_ msg_ptr) {
            DEBUGLOG("send message success");
        });
    });
}

void test_connect_client() {
    rocket::IPNetAddr::net_addr_sptr_t_ addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 22224);
    rocket::TCPClient client(addr);
    client.connect([addr]() {
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
    });
}

void test_connect() {
    // 调用 conenct 连接 server
    // wirte 一个字符串
    // 等待 read 返回结果
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        ERRORLOG("invalid fd %d", fd);
        exit(0);
    }
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(22224);
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    int rt = connect(fd, (sockaddr *) (&server_addr), sizeof(server_addr));
    DEBUGLOG("connect success");
    std::string msg = "hello rocket!";
    rt = write(fd, msg.c_str(), msg.length());
    DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());
    char buf[100];
    rt = read(fd, buf, 100);
    DEBUGLOG("success read %d bytes, [%s]", rt, std::string(buf).c_str());
}

int main() {
    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    // test_connect();
    // test_connect_client_and_write();
    // test_connect_client();

    test_connect_client_tinypb();
    // test_connect_client_and_write_and_read();
}
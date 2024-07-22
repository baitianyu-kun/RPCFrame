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
    test_connect_client();
}
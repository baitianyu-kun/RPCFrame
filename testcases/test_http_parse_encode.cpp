//
// Created by baitianyu on 2/8/25.
//
#include "net/protocol/http/http_define.h"
#include "net/protocol/http/http_parse.h"
#include "common/log.h"

using namespace rocket;

int main() {

//    Config::SetGlobalConfig("../conf/rocket.xml");
//    Logger::InitGlobalLogger(0);
//
//    std::string str = "POST /mysite/index.html HTTP/1.0\r\n"
//                      "Host: 10.101.101.10\r\n"
//                      "Content-Length: 30\r\n"
//                      "Content-Type: text/html;charset=utf-8\r\n"
//                      "\r\n"
//                      "username=zhangsan&password=123";
//
//    HTTPRequestParser requestParser;
//    requestParser.parse(str);
//    auto req = requestParser.getRequest();
//
//
//    std::string str2 = "HTTP/1.1 200 OK\r\n"
//                       "Server: Apache-Coyote/1.1\r\n"
//                       "Content-Type: text/html;charset=UTF-8\r\n"
//                       "Content-Length: 30\r\n"
//                       "\r\n"
//                       "username=zhangsan&password=123";
//    HTTPResponseParser responseParser;
//    responseParser.parse(str2);
//    auto res = responseParser.getResponse();
//    printf("%s",res.toString().c_str());
}
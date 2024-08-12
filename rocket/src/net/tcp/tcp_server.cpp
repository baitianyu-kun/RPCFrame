//
// Created by baitianyu on 7/20/24.
//
#include "net/tcp/tcp_server.h"
#include "net/coder/tinypb/tinypb_coder.h"
#include "net/coder/http/http_coder.h"
#include "common/msg_id_util.h"
#include "common/string_util.h"

namespace rocket {
    rocket::TCPServer::TCPServer(NetAddr::net_addr_sptr_t_ local_addr, NetAddr::net_addr_sptr_t_ register_center_addr,
                                 ProtocolType protocol /*ProtocolType::TinyPB_Protocol*/) : m_local_addr(
            local_addr), m_register_center_addr(register_center_addr), m_protocol_type(protocol) {
        init();
        INFOLOG("rocket TcpServer listen success on [%s]", m_local_addr->toString().c_str());
    }

    TCPServer::~TCPServer() {
        DEBUGLOG("~TCPServer");
    }

    void TCPServer::start() {
        m_io_thread_pool->start();
        // 注册到注册中心并在client的connect方法中启动eventloop
        registerToCenterAndStartLoop();

        // main event loop是主线程的epoll，但是每个子线程也各自拥有一个epoll，即各自拥有一个eventloop，整个是个这样的模式
        // 是一个主从reactor模式

        // 由于tcp server和tcp client用的是同一个event loop，
        // 所以在tcp client主动连接注册中心时候connect方法已经启动了eventloop，所以这里就不用启动了
        // 感觉这里设计的有点狗屎
        // m_main_event_loop->loop();
    }

    // 初始化各种东西
    void TCPServer::init() {
        // 接受器
        m_acceptor = std::make_shared<TCPAcceptor>(m_local_addr);
        m_main_event_loop = EventLoop::GetCurrentEventLoop();
        m_io_thread_pool = std::move(std::unique_ptr<IOThreadPool>(new IOThreadPool(MAX_THREAD_POOL_SIZE)));
        // 获取监听套接字，监听到上面有可读事件就执行onAccept事件
        m_listen_fd_event = std::make_shared<FDEvent>(m_acceptor->getListenFD());
        // 需要传递一个this指针过去指明对象，也不能TCPServer::onAccept，因为onAccept不是static的，所以前面加&
        // bind绑定类成员函数时，第一个参数表示对象的成员函数的指针，第二个参数表示对象的地址。
        // 必须显式地指定&Base::diplay_sum，因为编译器不会将对象的成员函数隐式转换成函数指针，所以必须在Base::display_sum前添加&；
        // 使用对象成员函数的指针时，必须要知道该指针属于哪个对象，因此第二个参数为对象的地址 &base或者this；
        m_listen_fd_event->listen(FDEvent::IN_EVENT, std::bind(&TCPServer::onAccept, this));
        m_main_event_loop->addEpollEvent(m_listen_fd_event);

        m_clear_client_timer_event = std::make_shared<TimerEventInfo>(TIMER_EVENT_INTERVAL, true,
                                                                      std::bind(&TCPServer::clearClientTimerFunc,
                                                                                this));
        m_main_event_loop->addTimerEvent(m_clear_client_timer_event);
        // dispatcher
        if (m_protocol_type == ProtocolType::HTTP_Protocol) {
            m_dispatcher = std::make_shared<HTTPDispatcher>();
            m_coder = std::make_shared<HTTPCoder>();
        } else if (m_protocol_type == ProtocolType::TinyPB_Protocol) {
            m_dispatcher = std::make_shared<TinyPBDispatcher>();
            m_coder = std::make_shared<TinyPBCoder>();
        }
    }

    void TCPServer::registerService(TinyPBDispatcher::protobuf_service_sptr_t_ service) {
        if (m_protocol_type == ProtocolType::TinyPB_Protocol) {
            std::dynamic_pointer_cast<TinyPBDispatcher>(m_dispatcher)->registerService(service);
        } else if (m_protocol_type == ProtocolType::HTTP_Protocol) {
            std::dynamic_pointer_cast<HTTPDispatcher>(m_dispatcher)->registerService(service);
        } else {
            ERRORLOG("unknown register service");
            exit(0);
        }
    }

    void TCPServer::onAccept() {
        auto ret = m_acceptor->accept();
        auto client_fd = ret.first;
        auto peer_addr = ret.second;
        m_client_counts++; // 已经连接的子线程个数

        // 轮询插入io thread中
        auto &io_thread = m_io_thread_pool->getIOThread();
        // 创建connection处理读写
        auto connection = std::make_shared<TCPConnection>(
                io_thread->getEventLoop(),
                client_fd,
                MAX_TCP_BUFFER_SIZE,
                peer_addr,
                m_local_addr,
                m_coder,
                m_dispatcher,
                TCPConnectionType::TCPConnectionByServer,
                m_protocol_type
        );
        connection->setState(Connected);
        m_client_connection.insert(connection);
        // 轮询添加到线程池中的线程中
        INFOLOG("TcpServer succeed get client, fd: %d, peer addr: %s, now client counts: %d", client_fd,
                peer_addr->toString().c_str(), m_client_counts);
    }

    void TCPServer::clearClientTimerFunc() {
        // 清理过期的connection
        // 1. 该connection不为nullptr
        // 2. 该connection的引用计数大于0，证明还有没释放的，感觉这个不是很必要
        // 3. 该connection的state为closed
        // 则该connection的状态为过期的
        auto iter = m_client_connection.begin();
        for (; iter != m_client_connection.end();) {
            if ((*iter) != nullptr && (*iter).use_count() > 0 && (*iter)->getState() == TCPState::Closed) {
                // erase的返回值是指向被删除元素的后继元素的迭代器
                iter = m_client_connection.erase(iter);
            } else {
                iter++;
            }
        }
    }

    void TCPServer::registerToCenterAndStartLoop() {
        auto all_service_names = std::dynamic_pointer_cast<HTTPDispatcher>(m_dispatcher)->getAllServiceName();
        std::string all_service_names_str = "";
        for (const auto &item: all_service_names) {
            all_service_names_str += item;
            all_service_names_str += ",";
        }
        all_service_names_str = all_service_names_str.substr(0, all_service_names_str.length() - 1);

        std::string msg_id = MSGIDUtil::GenerateMSGID();
        std::string final_req = "is_server:" + std::to_string(true) + g_CRLF
                                + "server_ip:" + m_local_addr->getStringIP() + g_CRLF
                                + "server_port:" + m_local_addr->getStringPort() + g_CRLF
                                + "method_full_name:" + all_service_names_str + g_CRLF
                                + "msg_id:" + msg_id;

        // DEBUGLOG("========final_res: %s========", final_res.c_str());

        auto req_protocol = std::make_shared<HTTPRequest>();
        req_protocol->m_request_body = final_req;
        req_protocol->m_request_method = HTTPMethod::POST;
        req_protocol->m_request_version = "HTTP/1.1";
        req_protocol->m_request_path = "/";
        req_protocol->m_request_properties.m_map_properties["Content-Length"] = std::to_string(final_req.length());
        req_protocol->m_request_properties.m_map_properties["Content-Type"] = content_type_text;
        req_protocol->m_msg_id = msg_id;

        auto client = std::make_shared<TCPClient>(m_register_center_addr, m_protocol_type);
        // 这里拿着this_tcp_server的共享指针无法释放，因为tcp server会一直保持运行，导致client无法析构，造成内存泄露
        client->connect([&client, req_protocol]()mutable {
            client->writeMessage(req_protocol,
                                 [](
                                         AbstractProtocol::abstract_pro_sptr_t_ msg)mutable {
                                 });
            client->readMessage(req_protocol->m_msg_id,
                                [&client](
                                        AbstractProtocol::abstract_pro_sptr_t_ msg)mutable {
                                    auto rsp_protocol = std::dynamic_pointer_cast<HTTPResponse>(
                                            msg);
                                    std::unordered_map<std::string, std::string> response_body_map;
                                    splitStrToMap(rsp_protocol->m_response_body, g_CRLF,
                                                  ":", response_body_map);
                                    rsp_protocol->m_msg_id = response_body_map["msg_id"];

                                    INFOLOG("%s | success register to center, rsp_protocol_body [%s], peer addr [%s], local addr[%s]",
                                            rsp_protocol->m_msg_id.c_str(),
                                            rsp_protocol->m_response_body.c_str(),
                                            client->getPeerAddr()->toString().c_str(),
                                            client->getLocalAddr()->toString().c_str());

                                    // client和tcp server中由于都是主线程，所以共用一个eventloop，即eventloop的引用计数为2，分别为client和tcp server在引用
                                    // 此时不能stop client里面的eventloop，否则tcp server也会退出
                                    // 只能把client里面的eventloop的共享指针给销毁了。
                                    // client里面是connection在使用eventloop，所以只需要把client里面的connection给销毁了就行
                                    // 然后eventloop的共享指针就会自动销毁。然后再手动释放client，达到client运行完成后自动释放client和其所有成员。

                                    // 手动析构connection对象，其里面的eventloop的共享指针自然会自动析构，loop函数也就停下来了
                                    client->getConnectionRef().reset();
                                    client.reset(); // 前面lambda捕获client引用，达到再回调函数中销毁client的目的
                                });
        });
    }
}



//
// Created by baitianyu on 8/11/24.
//
#include "net/register/register_center.h"
#include "net/coder/http/http_coder.h"
#include "common/msg_id_util.h"
#include "common/string_util.h"

namespace rocket {
    rocket::RegisterCenter::RegisterCenter(NetAddr::net_addr_sptr_t_ local_addr,
                                           ProtocolType protocol/*ProtocolType::TinyPB_Protocol*/) :
            m_local_addr(local_addr), m_protocol_type(protocol) {
        init();
        INFOLOG("rocket RegisterCenter listen success on [%s]", m_local_addr->toString().c_str());
    }

    RegisterCenter::~RegisterCenter() {
        DEBUGLOG("~RegisterCenter");
    }

    void RegisterCenter::start() {
        m_io_thread_pool->start();
        m_main_event_loop->loop();
    }

    void RegisterCenter::init() {
        m_acceptor = std::make_shared<TCPAcceptor>(m_local_addr);
        m_main_event_loop = EventLoop::GetCurrentEventLoop();
        m_io_thread_pool = std::move(std::unique_ptr<IOThreadPool>(new IOThreadPool(MAX_THREAD_POOL_SIZE)));
        m_listen_fd_event = std::make_shared<FDEvent>(m_acceptor->getListenFD());
        m_listen_fd_event->listen(FDEvent::IN_EVENT, std::bind(&RegisterCenter::onAccept, this));
        m_main_event_loop->addEpollEvent(m_listen_fd_event);
        m_clear_client_timer_event = std::make_shared<TimerEventInfo>(TIMER_EVENT_INTERVAL, true,
                                                                      std::bind(&RegisterCenter::clearClientTimerFunc,
                                                                                this));
        m_main_event_loop->addTimerEvent(m_clear_client_timer_event);

        m_update_server_timer_event = std::make_shared<TimerEventInfo>(UPDATE_SERVER_TIMER_EVENT_INTERVAL, true,
                                                                       std::bind(&RegisterCenter::updateServerMethod,
                                                                                 this));
        m_main_event_loop->addTimerEvent(m_update_server_timer_event);

        m_dispatcher = std::make_shared<RegisterDispatcher>();
        m_coder = std::make_shared<HTTPCoder>();
    }

    void RegisterCenter::onAccept() {
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
        INFOLOG("RegisterCenter succeed get client, fd: %d, peer addr: %s, now client counts: %d", client_fd,
                peer_addr->toString().c_str(), m_client_counts);
    }

    void RegisterCenter::clearClientTimerFunc() {
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

    void RegisterCenter::updateServerMethod() {
        // 注册中心请求服务端: update, msg_id 服务端返回：update, method_full_name，msg_id
        // 客户端请求注册中心：is_server, method_full_name，msg_id 返回：success, server_ip, server_port, msg_id
        // 服务端请求注册中心：is_server, method_full_name，msg_id 返回：add, msg_id
        std::string final_res = "update:" + std::to_string(false) + g_CRLF
                                + "msg_id:" + MSGIDUtil::GenerateMSGID();

        auto req_protocol = std::make_shared<HTTPRequest>();
        req_protocol->m_request_body = final_res;
        req_protocol->m_request_method = HTTPMethod::POST;
        req_protocol->m_request_version = "HTTP/1.1";
        req_protocol->m_request_path = "/";
        req_protocol->m_request_properties.m_map_properties["Content-Length"] = std::to_string(final_res.length());
        req_protocol->m_request_properties.m_map_properties["Content-Type"] = content_type_text;

        auto all_server_list = m_dispatcher->get_all_server_list();
        auto this_register_center = shared_from_this();
        for (const auto &server: all_server_list) {
            m_client = std::make_shared<TCPClient>(server, m_protocol_type);
            m_client->connect([this_register_center, req_protocol]() {
                this_register_center->GetClient()->writeMessage(req_protocol,
                                                                [this_register_center](
                                                                        AbstractProtocol::abstract_pro_sptr_t_ msg) {
                                                                });
                this_register_center->GetClient()->readMessage(req_protocol->m_msg_id,
                                                               [this_register_center](
                                                                       AbstractProtocol::abstract_pro_sptr_t_ msg) {
                                                                   auto rsp_protocol = std::dynamic_pointer_cast<HTTPResponse>(
                                                                           msg);
                                                                   std::unordered_map<std::string, std::string> response_body_map;
                                                                   splitStrToMap(rsp_protocol->m_response_body, g_CRLF,
                                                                                 ":", response_body_map);
                                                                   INFOLOG("%s | success update, rsp_protocol_body [%s], peer addr [%s], local addr[%s]",
                                                                           rsp_protocol->m_msg_id.c_str(),
                                                                           rsp_protocol->m_response_body.c_str(),
                                                                           this_register_center->GetClient()->getPeerAddr()->toString().c_str(),
                                                                           this_register_center->GetClient()->getLocalAddr()->toString().c_str())
                                                               });
            });
        }

    }

    TCPClient *RegisterCenter::GetClient() {
        return m_client.get();
    }
}



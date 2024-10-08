//
// Created by baitianyu on 7/20/24.
//
#include <unistd.h>
#include "net/tcp/tcp_connection.h"
#include "net/coder/tinypb/tinypb_coder.h"
#include "net/coder/http/http_response.h"

namespace rocket {

    rocket::TCPConnection::TCPConnection(EventLoop::event_loop_sptr_t_ event_loop,
                                         int client_fd,
                                         int buffer_size,
                                         NetAddr::net_addr_sptr_t_ peer_addr,
                                         NetAddr::net_addr_sptr_t_ local_addr,
                                         AbstractCoder::abstract_coder_sptr_t_ coder,
                                         AbstractDispatcher::abstract_disp_sptr_t dispatcher,
                                         TCPConnectionType type /*TCPConnectionByServer*/,
                                         ProtocolType protocol /*ProtocolType::TinyPB_Protocol*/) :
            m_event_loop(event_loop),
            m_local_addr(local_addr),
            m_peer_addr(peer_addr),
            m_state(NotConnected),
            m_client_fd(client_fd),
            m_connection_type(type),
            m_coder(coder),
            m_dispatcher(dispatcher),
            m_protocol_type(protocol) {
        m_in_buffer = std::make_shared<TCPBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TCPBuffer>(buffer_size);
        m_fd_event = FDEventPool::GetFDEventPool()->getFDEvent(client_fd);
        m_fd_event->setNonBlock();
        if (m_connection_type == TCPConnectionByServer) {
            listenRead(); // 本方是服务器的话，监听读
        }
    }

    TCPConnection::~TCPConnection() {
        DEBUGLOG("~TCPConnection");
    }

    void TCPConnection::onRead() {
        if (m_state != Connected) {
            ERRORLOG("onRead error, client has already disconneced, addr[%s], clientfd[%d]",
                     m_peer_addr->toString().c_str(), m_client_fd);
            return;
        }
        // 是否从socket上读取并全部写入到in buffer中
        bool is_read_and_write_all = false;
        bool is_close = false;
        while (!is_read_and_write_all) {
            // 没有读完就一直读，没有写入位置了就扩充
            if (m_in_buffer->writeAbleSize() == 0) {
                m_in_buffer->resizeBuffer(2 * m_in_buffer->getBufferSize());
            }
            int write_count = m_in_buffer->writeAbleSize();
            int write_index = m_in_buffer->getWriteIndex();
            // 返回读取到的字节数
            int ret = read(m_client_fd, &(m_in_buffer->getRefBuffer()[write_index]), write_count);
            DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", ret, m_peer_addr->toString().c_str(),
                     m_client_fd);
            if (ret > 0) {
                // 将write指针向后移动刚刚读取的字节数个位置
                // 比如你要求从里面读取100byte，但是ret返回了50个，说明只有50个可以读了，所以ret < write count的话证明已经读完了
                m_in_buffer->moveWriteIndex(ret);
                if (ret == write_count) {
                    continue;
                } else if (ret < write_count) {
                    is_read_and_write_all = true;
                    break;
                }
            } else if (ret == 0) {
                is_close = true;
                break;
            } else if (ret == -1 && errno == EAGAIN) {
                // 以 O_NONBLOCK的标志打开文件/socket/FIFO，如果连续做read操作而没有数据可读。
                // 此时程序不会阻塞起来等待数据准备就绪返回，read函数会返回一个错误EAGAIN，提示你的应用程序现在没有数据可读请稍后再试。
                is_read_and_write_all = true;
                break;
            }
        }
        if (is_close) {
            INFOLOG("peer closed, peer addr [%s], clientfd [%d]", m_peer_addr->toString().c_str(), m_client_fd);
            clear(); // 取消各种读写监听以及删除当前的m_fd_event
            return;
        }
        if (!is_read_and_write_all) {
            ERRORLOG("not read all data, peer addr [%s], clientfd [%d]", m_peer_addr->toString().c_str(), m_client_fd);
        }
        // 执行rpc任务，即execute，下面先简单的读取一下
        execute();
    }

    /* void TCPConnection::execute() {
        // 将rpc请求执行业务逻辑，获取rpc响应，再把rpc响应发送回去
        std::vector<char> tmp;
        auto size = m_in_buffer->readAbleSize();
        tmp.resize(size);
        m_in_buffer->readFromBuffer(tmp, size);

        std::string tmp_msg;
        for (const auto &item: tmp) {
            tmp_msg += item;
        }
        tmp_msg += '\0'; // 需要加一个结束符，否则会输出其他字符
        m_out_buffer->writeToBuffer(tmp_msg.c_str(), tmp_msg.length());
        INFOLOG("success get from client[%s], data[%s]", m_peer_addr->toString().c_str(), tmp_msg.c_str());
        // EPOLLOUT三种应用场景
        // 客户端连接场景
        // 触发条件：客户端connect上服务端后，服务端得到fd，这时候把fd添加到epoll 事件池里面后，因为连接可写，会触发EPOLLOUT事件，所以一connect就会触发EPOLLOUT但是我们只希望
        // 执行完成RPC之后再去触发，所以需要在这里进行添加事件，写完后立即取消EPOLLOUT，否则会因为客户端还连着，连接可写，导致再次无限次触发EPOLLOUT事件

        // 客户端发包场景
        // 触发条件：缓冲区从满到不满，会触发EPOLLOUT事件
        // 典型应用场景(数据包发送问题)：
        // 数据包发送逻辑：将数据包发完内核缓冲区–>进而由内核再将缓冲区的内容发送出去；这边send只是做了第一部分的工作，如果缓存区满的话send将会得到已发送的数据大小(成功放到缓冲区的)，而不是整个数据包大小。
        // 这种情况我们可以借助EPOLLOUT事件加以解决：如果send部分成功，则表示缓存区满了，那么把剩下的部分交给epoll，当检测到EPOLLOUT事件后，再将剩余的包发送出去。

        // 重新注册EPOLLOUT事件
        // 触发条件：如果当连接可用后，且缓存区不满的情况下，调用epoll_ctl将fd重新注册到epoll事件池(使用EPOLL_CTL_MOD)，这时也会触发EPOLLOUT时间。
        // 典型应用场景：
        // send或write发包函数会涉及系统调用，存在一定开销，如果能将数据包聚合起来，然后调用writev将多个数据包一并发送，则可以减少系统调用次数，提高效率。
        // 这时EPOLLOUT事件就派上用场了：当发包时，可以将先数据包发到数据buffer(用户缓存区)中存放，
        // 然后通过重新注册EPOLLOUT事件，从而触发EPOLLOUT事件时，再将数据包一起通过writev发送出去。

        // 注册写事件，写完之后记得立即取消，否则会一直触发
        listenWrite();
    } */

    void TCPConnection::execute() {
        // 1. 作为服务端使用
        //    将RPC请求执行业务逻辑，获取RPC响应，再把RPC响应发送回去
        // 2. 作为客户端使用
        //    从接收缓冲区in buffer中接收数据，decode为message对象，如果有其回调函数的话，执行其回调
        //    (作为客户端的写事件回调在onWrite中执行，这里是执行其读取的回调read dones)
        if (m_connection_type == TCPConnectionByServer) {
            std::vector<AbstractProtocol::abstract_pro_sptr_t_> results;
            std::vector<AbstractProtocol::abstract_pro_sptr_t_> reply_messages;
            m_coder->decode(results, m_in_buffer);
            for (const auto &result: results) {
                INFOLOG("success get request[%s] from client[%s]", result->m_msg_id.c_str(),
                        m_peer_addr->toString().c_str());
                AbstractProtocol::abstract_pro_sptr_t_ message{nullptr};
                if (m_protocol_type == ProtocolType::HTTP_Protocol) {
                    message = std::make_shared<HTTPResponse>();
                } else if (m_protocol_type == ProtocolType::TinyPB_Protocol) {
                    message = std::make_shared<TinyPBProtocol>();
                }
                // 当一个类被共享智能指针 share_ptr 管理，且在类的成员函数里需要把当前类对象作为参数传给其他函数时，这时就需要传递一个指向自身的 share_ptr。
                // RPCDispatcher::GetRPCDispatcher()->dispatch(result, message, shared_from_this());
                m_dispatcher->dispatch(result, message, m_peer_addr, m_local_addr);
                reply_messages.emplace_back(message);
            }
            m_coder->encode(reply_messages, m_out_buffer);
            listenWrite();
        } else {
            std::vector<AbstractProtocol::abstract_pro_sptr_t_> results;
            m_coder->decode(results, m_in_buffer, true);
            for (const auto &result: results) {
                auto iter = m_read_dones.find(result->m_msg_id);
                if (iter != m_read_dones.end()) {
                    std::function<void(AbstractProtocol::abstract_pro_sptr_t_)> done = iter->second;
                    // tcp server中的client->connect那块的三个lambda表达式嵌套的问题
                    // 先保存一下回调，然后在调用，不然如果在调用函数里删除connection的指针的话，则删除后整个tcp connection全部析构，即m_read_dones也析构了
                    // 导致m_read_dones在上一层lambda表达式调用时提示空指针。所以可以先保存一下这个可调用函数，然后删除m_read_dones，然后在调用done，
                    // 这样无论在done里面删除整个tcp connection类或者是进行其他操作都不会出问题了。
                    m_read_dones.erase(iter);
                    done(result);
                }else{
                    DEBUGLOG("not found result->m_msg_id: %s",result->m_msg_id.c_str());
                }
            }
        }
    }


    void TCPConnection::onWrite() {
        if (m_state != Connected) {
            ERRORLOG("onWrite error, client has already disconnected, addr[%s], clientfd[%d]",
                     m_peer_addr->toString().c_str(), m_client_fd);
            clear();
            return;
        }
        // 当前作为客户端的情况下
        if (m_connection_type == TCPConnectionByClient) {
            // 将数据写入到buffer里面，然后全部发送
            // 1. 将msg encode得到字节流
            // 2. 将字节流写入到buffer里面然后全部发送
            std::vector<AbstractProtocol::abstract_pro_sptr_t_> messages;
            messages.reserve(m_write_dones.size());
            for (const auto &write_done: m_write_dones) {
                messages.emplace_back(write_done.first);
            }
            m_coder->encode(messages, m_out_buffer, true);
        }
        // 是否从outbuffer中全部读取并发送给socket
        bool is_read_and_write_all = false;
        while (!is_read_and_write_all) {
            if (m_out_buffer->readAbleSize() == 0) {
                DEBUGLOG("no more data need to send to client [%s]", m_peer_addr->toString().c_str());
                is_read_and_write_all = true;
                break;
            }
            int read_count = m_out_buffer->readAbleSize();
            int read_index = m_out_buffer->getReadIndex();
            int ret = write(m_client_fd, &(m_out_buffer->getRefBuffer()[read_index]), read_count);
            if (ret >= read_count) {
                DEBUGLOG("no more data need to send to client [%s]", m_peer_addr->toString().c_str());
                is_read_and_write_all = true;
                break;
            } else if (ret == -1 && errno == EAGAIN) {
                // 发送缓冲区已经满了，已经不可以继续写入
                // 等下一次再发送
                ERRORLOG("write data error, errno == EAGIN and rt == -1, may be send buffer is full");
                break;
            }
        }
        if (is_read_and_write_all) {
            // 取消监听fd event，写完以后得去除掉，否则会一直进行触发，写只需要发送数据时候才需要监听，即只在execute中添加进行监听
            m_fd_event->cancel_listen(FDEvent::OUT_EVENT);
            m_event_loop->addEpollEvent(m_fd_event);
        }

        // 作为客户端
        // 发送完成后，需要调用m write dones的回调
        if (m_connection_type == TCPConnectionByClient) {
            for (const auto &write_done: m_write_dones) {
                write_done.second(write_done.first);
            }
            m_write_dones.clear(); // 清空回调函数
        }
    }

    void TCPConnection::pushSendMessage(const AbstractProtocol::abstract_pro_sptr_t_ &message,
                                        const std::function<void(AbstractProtocol::abstract_pro_sptr_t_)> &done) {
        m_write_dones.emplace_back(message, done);
    }

    void TCPConnection::pushReadMessage(const std::string &msg_id,
                                        const std::function<void(AbstractProtocol::abstract_pro_sptr_t_)> &done) {
        m_read_dones.emplace(msg_id, done);
    }

    void TCPConnection::setState(TCPState new_state) {
        m_state = new_state;
    }

    TCPState TCPConnection::getState() {
        return m_state;
    }

    // 服务器被动关闭
    void TCPConnection::clear() {
        // 取消所有监听，并设置state
        if (m_state == Closed) {
            return;
        }
        m_fd_event->cancel_listen(FDEvent::IN_EVENT);
        m_fd_event->cancel_listen(FDEvent::OUT_EVENT);
        m_event_loop->deleteEpollEvent(m_fd_event);
        m_state = Closed;
        close(m_fd_event->getFD());
        close(m_client_fd);
    }

    int TCPConnection::getFD() {
        return m_client_fd;
    }

    // 服务器主动关闭连接
    // 防止恶意tcp连接长时间连接但是不收发数据，所以需要主动去进行关闭
    void TCPConnection::shutdown() {
        if (m_state == Closed || m_state == NotConnected) {
            return;
        }
        // 设置为处于半关闭状态
        m_state = HalfClosing;
        // 恶意tcp长时间连接但是不收发数据，即在上面的onRead里面，socket有事件触发，但是read读取的字节数为0，就代表没有数据可读了，就可以进行关闭
        // 需要服务器主动去关闭，这个时候服务端就相当于客户端了，主动触发四次挥手，即第一次发送FIN包

        // 调用 shutdown 关闭读写，意味着服务器不会再对这个 fd 进行读写操作了
        // 发送 FIN 报文， 触发了四次挥手的第一个阶段
        // 当 fd 发生可读事件，但是可读的数据为0，即 对端发送了 FIN

        // SHUT_RDWR：同时断开 I/O 流。相当于分两次调用 shutdown()，其中一次以 SHUT_RD 为参数，另一次以 SHUT_WR 为参数。
        // shutdown() 用来关闭连接，而不是套接字，不管调用多少次 shutdown()，套接字依然存在，直到调用 close() / closesocket() 将套接字从内存清除。

        // 调用 close()/closesocket() 关闭套接字时，或调用 shutdown() 关闭输出流时，都会向对方发送 FIN 包。FIN 包表示数据传输完毕，计算机收到 FIN 包就知道不会再有数据传送过来了。
        // 默认情况下，close()/closesocket() 会立即向网络中发送FIN包，不管输出缓冲区中是否还有数据，而shutdown() 会等输出缓冲区中的数据传输完毕再发送FIN包。
        // 也就意味着，调用 close()/closesocket() 将丢失输出缓冲区中的数据，而调用 shutdown() 不会。
        ::shutdown(m_client_fd, SHUT_RDWR);
    }

    void TCPConnection::setConnectionType(const TCPConnectionType new_type) {
        m_connection_type = new_type;
    }

    void TCPConnection::listenWrite() {
        m_fd_event->listen(FDEvent::OUT_EVENT, std::bind(&TCPConnection::onWrite, this));
        m_event_loop->addEpollEvent(m_fd_event);
    }

    void TCPConnection::listenRead() {
        // 监听的时候，callback为on read函数
        // auto newCallable = bind(callable, arg_list);
        // 该形式表达的意思是：当调用newCallable时，会调用callable，并传给它arg_list中的参数。
        // 类成员函数较为特殊，需要传入一个this指针指定是哪个对象
        // 非静态类成员函数需要&类::成员函数，静态的直接类::成员函数，即可

        // bind绑定类成员函数时，第一个参数表示对象的成员函数的指针，第二个参数表示对象的地址，
        // 这是因为对象的成员函数需要有this指针。
        // 并且编译器不会将对象的成员函数隐式转换成函数指针，需要通过&手动转换；
        // 对于非类成员函数，编译器会把函数名隐式转换成指针的形式，即转换成函数指针，而对于类成员函数无法转换

        // 但是静态的由于是独立于对象的，可以看作非类成员函数，
        // 编译器会把函数名隐式转换成指针的形式，所以直接赋给其就可以
        // std::bind的返回值是可调用实体，可以直接赋给std::function。
        m_fd_event->listen(FDEvent::IN_EVENT, std::bind(&TCPConnection::onRead, this));
        m_event_loop->addEpollEvent(m_fd_event);
    }

    NetAddr::net_addr_sptr_t_ TCPConnection::getLocalAddr() {
        return m_local_addr;
    }

    NetAddr::net_addr_sptr_t_ TCPConnection::getPeerAddr() {
        return m_peer_addr;
    }

}




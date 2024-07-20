//
// Created by baitianyu on 7/20/24.
//

#ifndef RPCFRAME_TCP_CONNECTION_H
#define RPCFRAME_TCP_CONNECTION_H

#include <memory>
#include <map>
#include <queue>
#include "net/tcp/net_addr.h"
#include "net/tcp/tcp_buffer.h"
#include "net/io_thread.h"

namespace rocket {
    enum TCPState {
        NotConnected = 1,
        Connected = 2,
        HalfClosing = 3,
        Closed = 4
    };

    enum TCPConnectionType {
        TCPConnectionByServer = 1,  // 作为服务端使用，代表跟对端客户端的连接
        TCPConnectionByClient = 2,  // 作为客户端使用，代表跟对端服务端的连接
    };

    class TCPConnection {
    public:
        using tcp_connection_sptr_t_ = std::shared_ptr<TCPConnection>;
    public:
        TCPConnection();

        ~TCPConnection();

        // 读取数据，执行，并写入结果三个步骤
        void onRead();

        void execute();

        void onWrite();

        void setState(const TCPState new_state);

        TCPState getState();

        void clear();

        int getFD();

        // 服务器主动关闭连接
        void shutdown();

        void setConnectionType(TCPConnectionType type);

        // 监听可写事件
        void listenWrite();

        // 监听可读事件
        void listenRead();


    };

}


#endif //RPCFRAME_TCP_CONNECTION_H

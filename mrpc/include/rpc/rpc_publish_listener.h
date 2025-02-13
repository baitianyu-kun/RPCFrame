//
// Created by baitianyu on 2/12/25.
//

#ifndef RPCFRAME_RPC_PUBLISH_LISTENER_H
#define RPCFRAME_RPC_PUBLISH_LISTENER_H

#include "net/tcp/tcp_server.h"

namespace mrpc {

    class PublishListener : public TCPServer {
    public:
        using ptr = std::shared_ptr<PublishListener>;

        PublishListener()
    };
}

#endif //RPCFRAME_RPC_PUBLISH_LISTENER_H

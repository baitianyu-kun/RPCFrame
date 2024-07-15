//
// Created by baitianyu on 7/14/24.
//

#ifndef RPCFRAME_EVENTLOOP_H
#define RPCFRAME_EVENTLOOP_H
#include <pthread.h>
#include <set>

namespace rocket{

    class EventLoop{
    public:
        EventLoop();
        ~EventLoop();

        void loop();
        void wakeup();
        void stop();
    private:
        // event loop每个线程只能有一个，线程号
        pid_t m_pid;
        std::set<int> m_listen_fds;
    };

}

#endif //RPCFRAME_EVENTLOOP_H

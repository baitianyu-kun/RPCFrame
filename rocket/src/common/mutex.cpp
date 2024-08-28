//
// Created by baitianyu on 8/28/24.
//
#include "common/mutex.h"
#include "common/log.h"

namespace rocket {

    CoroutineMutex::CoroutineMutex() {

    }

    CoroutineMutex::~CoroutineMutex() {
        if (m_lock) {
            unlock(); // 析构前记得解锁
        }
    }

    void CoroutineMutex::lock() {
        if (Coroutine::IsMainCoroutine()) {
            ERRORLOG("main coroutine can't use coroutine mutex");
            return;
        }
        auto cur_cor = Coroutine::GetCurrentCoroutine();
        ScopeMutext<Mutex> lock(m_mutex);
        if (!m_lock) {
            // 没有锁定的话
            m_lock = true;
            DEBUGLOG("coroutine succeed get coroutine mutex");
            lock.unlock();
        } else {
            // 已经锁定的话，即说明已经有协程拿了锁，所以需要放入等待队列里面，等待其他释放锁
            m_sleep_cors.emplace(cur_cor);
            auto sleep_cors_size = m_sleep_cors.size();
            lock.unlock();
            // 等待协程互斥锁，当前睡眠队列存在多少个协程
            DEBUGLOG("coroutine yield, pending coroutine mutex, current sleep queue exist [%d] coroutines",
                     sleep_cors_size);
            Coroutine::Yield(); // 挂起协程后，从用户协程回到主协程，让出协程
        }
    }

    void CoroutineMutex::unlock() {
        if (Coroutine::IsMainCoroutine()) {
            ERRORLOG("main coroutine can't use coroutine mutex");
            return;
        }
        ScopeMutext<Mutex> lock(m_mutex);
        if (m_lock) {
            // 释放当前锁，并且因为之前未获取成功的routine还在等待队列中，所以需要从等待队列中拿出头部重新加入调度
            // 当持有互斥锁的协程解锁时，就需要从等待队列中选择一个协程继续执行，以便让其他协程有机会竞争锁资源。
            // 通过从等待队列中取出一个协程并重新resume，可以实现协程的调度，让等待队列中的协程得以继续执行，从而实现多个协程之间的协作和资源共享。
            m_lock = false;
            if (m_sleep_cors.empty()) {
                return;
            }
            auto first_cor = m_sleep_cors.front();
            m_sleep_cors.pop();
            lock.unlock();
            if (first_cor) {
                DEBUGLOG("coroutine unlock, now to resume coroutine [%d]", first_cor->getCorId());
                // 放入eventloop中启动，将等待的协程重新加入调度
                EventLoop::GetCurrentEventLoop()->addTask([first_cor]() {
                    Coroutine::Resume(first_cor);
                }, true);
            }
        }
    }
}
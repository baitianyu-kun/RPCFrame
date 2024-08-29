//
// Created by baitianyu on 7/14/24.
//

#ifndef RPCFRAME_MUTEX_H
#define RPCFRAME_MUTEX_H

#include <pthread.h>
#include <queue>
#include "coroutine/coroutine.h"

namespace rocket {

    template<class T>
    class ScopeMutext {

    public:
        ScopeMutext(T &mutex) : m_mutex(mutex) {
            m_mutex.lock();
            m_is_lock = true;
        }

        ~ScopeMutext() {
            m_mutex.unlock();
            m_is_lock = false;
        }

        void lock() {
            if (!m_is_lock) {
                m_mutex.lock();
            }
        }

        void unlock() {
            if (m_is_lock) {
                m_mutex.unlock();
            }
        }

    private:
        T &m_mutex;
        bool m_is_lock{false};

    };

    class Mutex {
    public:
        Mutex() {
            pthread_mutex_init(&m_mutex, NULL);
        }

        ~Mutex() {
            pthread_mutex_destroy(&m_mutex);
        }

        void lock() {
            pthread_mutex_lock(&m_mutex);
        }

        void unlock() {
            pthread_mutex_unlock(&m_mutex);
        }

        pthread_mutex_t *getMutex() {
            return &m_mutex;
        }

    private:
        pthread_mutex_t m_mutex;

    };

    class CoroutineMutex {
    public:
        CoroutineMutex();

        ~CoroutineMutex();

        void lock();

        void unlock();

        std::queue<Coroutine*>& getAllSleepCorsRef();

    private:
        bool m_lock{false};
        Mutex m_mutex;
        std::queue<Coroutine *> m_sleep_cors; // 等待队列
    };

}

#endif //RPCFRAME_MUTEX_H

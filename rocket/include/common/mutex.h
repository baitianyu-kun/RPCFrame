//
// Created by baitianyu on 7/14/24.
//

#ifndef RPCFRAME_MUTEX_H
#define RPCFRAME_MUTEX_H

#include <pthread.h>
#include <queue>

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

    template<class T>
    class ReadScopedLock {
    public:
        ReadScopedLock(T& mutex):m_mutex(mutex){
            m_mutex.rlock();
            m_locked = true;
        }
        ~ReadScopedLock(){
            unlock();
        }
        void lock(){
            if(!m_locked){
                m_mutex.rlock();
                m_locked = true;
            }
        }
        void unlock(){
            if(m_locked){
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T& m_mutex;
        bool m_locked = false;
    };

    template<class T>
    class WriteScopedLock {
    public:
        WriteScopedLock(T& mutex):m_mutex(mutex){
            m_mutex.wlock();
            m_locked = true;
        }
        ~WriteScopedLock(){
            unlock();
        }
        void lock(){
            if(!m_locked){
                m_mutex.wlock();
                m_locked = true;
            }
        }
        void unlock(){
            if(m_locked){
                m_mutex.unlock();
                m_locked = false;
            }
        }
    private:
        T& m_mutex;
        bool m_locked = false;
    };

    class RWMutex {
    public:
        using ReadLock = ReadScopedLock<RWMutex>;
        using WriteLock = WriteScopedLock<RWMutex>;
        RWMutex(){
            pthread_rwlock_init(&m_rwmutex,nullptr);
        }
        ~RWMutex(){
            pthread_rwlock_destroy(&m_rwmutex);
        }
        void rlock(){
            pthread_rwlock_rdlock(&m_rwmutex);
        }
        void wlock(){
            pthread_rwlock_wrlock(&m_rwmutex);
        }
        void unlock(){
            pthread_rwlock_unlock(&m_rwmutex);
        }
    private:
        pthread_rwlock_t m_rwmutex;
    };

}

#endif //RPCFRAME_MUTEX_H

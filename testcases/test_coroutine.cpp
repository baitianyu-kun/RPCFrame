//
// Created by baitianyu on 8/28/24.
//
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "coroutine/coroutine.h"
#include "common/log.h"

rocket::Coroutine::coroutine_sptr_t_ cor;

class Test {

public:
    rocket::CoroutineMutex m_coroutine_mutex;
    int a = 1;
};

Test test_;

void fun1() {
    std::cout << "cor1 ---- now first resume fun1 coroutine by thread 1" << std::endl;
    std::cout << "cor1 ---- now begin to yield fun1 coroutine" << std::endl;
    rocket::Coroutine::Yield();
    std::cout << "cor1 ---- fun1 coroutine back, now end" << std::endl;
}

void *thread_fun1(void *) {
    int stack_size = 128 * 1024;
    char *sp = reinterpret_cast<char *>(malloc(stack_size));
    cor = std::make_shared<rocket::Coroutine>(stack_size, sp, fun1); // 创建用户协程，会自动创建主协程


    rocket::Coroutine::Resume(cor.get()); // fun1，直到yield停止
    std::cout << "main here in thread 1" << std::endl; // 开始执行这块
    rocket::Coroutine::Resume(cor.get()); // 继续执行yield后面的代码
    return nullptr;
}

int main() {

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger(0);

    pthread_t thread1;
    pthread_create(&thread1, nullptr, &thread_fun1, nullptr);
    pthread_join(thread1, nullptr);

    return 0;
}
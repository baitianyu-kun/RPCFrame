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

void fun1() {
    DEBUGLOG("cor1 ---- now first resume fun1 coroutine by thread 1");
    DEBUGLOG("cor1 ---- now begin to yield fun1 coroutine");
    rocket::Coroutine::Yield();
    DEBUGLOG("cor1 ---- fun1 coroutine back, now end");
}

void *main_thread_fun1(void *) {
    DEBUGLOG("main thread 1 begin");

    rocket::Coroutine::Resume(cor.get()); // fun1，直到yield停止
    // 此时该函数和main函数下面都是主线程，所以可以直接返回，如果要新建一个thread的话，则需要先初始化那个thread的main thread，才能跨线程调度主线程的协程
    // 可以见test_coroutine3.cpp
    DEBUGLOG("execute coroutines of the main thread, id = [%d]",cor->getCorId());
    rocket::Coroutine::Resume(cor.get()); // 继续执行yield后面的代码

    DEBUGLOG("main thread 1 end");
    return nullptr;
}

int main() {

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger(0);
    // 创建用户协程，会自动创建主协程，此时该协程是跑在主线程上面的
    int stack_size = 128 * 1024;
    char *sp = reinterpret_cast<char *>(malloc(stack_size));
    cor = std::make_shared<rocket::Coroutine>(stack_size, sp, fun1);

    DEBUGLOG("main thread create coroutine, id = [%d]",cor->getCorId());

    main_thread_fun1(nullptr);

    return 0;
}
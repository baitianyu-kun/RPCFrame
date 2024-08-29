//
// Created by baitianyu on 8/29/24.
//
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "coroutine/coroutine.h"
#include "common/log.h"

rocket::Coroutine::coroutine_sptr_t_ cor;

void fun1() {
    DEBUGLOG("cor1 ---- now first resume fun1 coroutine by thread 1");
    DEBUGLOG("cor1 ---- now begin to yield fun1 coroutine");
    rocket::Coroutine::Yield();
    DEBUGLOG("cor1 ---- fun1 coroutine back, now end");
}

// thread1跨线程调度主线程协程
void *thread1_func(void *) {
    DEBUGLOG("thread 1 begin");
    // 需要先初始化该thread的主协程，只有有了主协程才能继续调度，也就能跨当前线程thread1调度主线程的协程，这里直接初始化当前协程，如果当前协程的cur协程为nullptr
    // 说明还没初始化过，那么在初始化过程中会自动创建主协程，并把当前线程的cur协程设置为主协程
    rocket::Coroutine::GetCurrentCoroutine();

    rocket::Coroutine::Resume(cor.get());
    DEBUGLOG("execute coroutines of the main thread across threads, id = [%d]", cor->getCorId());
    rocket::Coroutine::Resume(cor.get());
    DEBUGLOG("thread 1 end");
    return nullptr;
}

int main() {

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger(0);
    // 创建用户协程，会自动创建主协程，此时该协程是跑在主线程上面的
    int stack_size = 128 * 1024;
    char *sp = reinterpret_cast<char *>(malloc(stack_size));
    cor = std::make_shared<rocket::Coroutine>(stack_size, sp, fun1);

    DEBUGLOG("main thread create coroutine, id = [%d]", cor->getCorId());

    pthread_t thread1;
    pthread_create(&thread1, nullptr, &thread1_func, nullptr);
    pthread_join(thread1, nullptr);

    return 0;
}
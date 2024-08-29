//
// Created by baitianyu on 8/29/24.
//
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include "coroutine/coroutine.h"
#include "common/log.h"
#include "net/eventloop.h"

rocket::Coroutine::coroutine_sptr_t_ cor;
rocket::Coroutine::coroutine_sptr_t_ cor2;

// 测试同步，创建两个协程fun1和fun2

class Test {
public:
    rocket::CoroutineMutex m_coroutine_mutex;
    int a = 1;
};

Test test_;

void fun1() {
    DEBUGLOG("cor1 ---- now resume fun1 coroutine");
    test_.m_coroutine_mutex.lock();
    DEBUGLOG("cor1 ---- coroutine lock on test_, sleep 5s begin");
    sleep(5);
    DEBUGLOG("cor1 ---- sleep 5s end, now back coroutine lock");
    test_.m_coroutine_mutex.unlock();
    DEBUGLOG("cor1 ---- now end");
}

// fun2在去获取mutex的时候，发现fun1已经占有了，所以fun2会挂起，让出协程到thread2的主协程中
// 即接下来会去执行 DEBUGLOG("thread 2 end");
// 当fun1解锁的时候，会去查看pending coroutine，然后把fun2给resume了
// 然后resume后，fun2可以正常执行lock及以下的语句，实现获取锁的功能

// 实现了获取锁失败后主动让出协程的功能，thread2执行fun2主动让出
// 协程并回到thread2的主协程后，会继续执行DEBUGLOG("thread 2 end");
// 不会阻塞thread2的执行

// 即fun2阻塞后，thread2继续执行，直到thread2结束。随后thread1在unlock时候
// 是跨线程到thread2去执行thread2里面的协程，即最后fun2去成功lock的时候，
// 可以看到是在fun1的那个线程thread1中进行的，而不是已经消亡的thread2.

// 从fun1里面的unlock要resume到fun2，但是fun1里已经处于非主协程的时候，无法进行resume
// 如果fun1先回到主协程，那么就会丢掉fun1里面的unlock里面的first_cor

// 所以需要thread1中的fun1先回到主协程，然后从主协程中去继续fun2
void fun2() {
    DEBUGLOG("cor2 ---- now resume fun2 coroutine");
    sleep(1);
    DEBUGLOG("cor2 ---- coroutine2 want to get coroutine lock of test_");
    test_.m_coroutine_mutex.lock();
    DEBUGLOG("cor2 ---- coroutine2 get coroutine lock of test_ success");
    test_.m_coroutine_mutex.unlock();
    DEBUGLOG("cor2 ---- now end");
}

void *thread1_func(void *) {
    DEBUGLOG("thread 1 begin");
    rocket::Coroutine::GetCurrentCoroutine();

    rocket::Coroutine::Resume(cor.get());
    DEBUGLOG("thread 1 end");
    // 在thread1的mutex中，在下面：
    // EventLoop::GetCurrentEventLoop()->addTask([first_cor]() {
    //                     Coroutine::Resume(first_cor);
    // }, true);
    // Coroutine::Yield();
    // 即unlock结束后，设置好回调函数(回到fun2去重新调度锁，即回到fun2的lock方法)后，先Yield回到主协程
    // 然后主协程开启eventloop，eventloop自动从主协程中回到func2中
    // 然后resume到fun2结束后，可选择自动关闭eventloop
    // 最后完成调度
    rocket::EventLoop::GetCurrentEventLoop()->loop();
    return nullptr;
}

void *thread2_func(void *) {
    DEBUGLOG("thread 2 begin");
    rocket::Coroutine::GetCurrentCoroutine();

    rocket::Coroutine::Resume(cor2.get());
    DEBUGLOG("thread 2 end");
    return nullptr;
}

int main() {

    rocket::Config::SetGlobalConfig("../conf/rocket.xml");
    rocket::Logger::InitGlobalLogger(0);
    // 创建用户协程，会自动创建主协程，此时该协程是跑在主线程上面的
    int stack_size = 128 * 1024;
    char *sp = reinterpret_cast<char *>(malloc(stack_size));
    cor = std::make_shared<rocket::Coroutine>(stack_size, sp, fun1);

    char *sp2 = reinterpret_cast<char *>(malloc(stack_size));
    cor2 = std::make_shared<rocket::Coroutine>(stack_size, sp2, fun2);

    DEBUGLOG("main thread create coroutine, id = [%d]", cor->getCorId());
    DEBUGLOG("main thread create coroutine, id = [%d]", cor2->getCorId());

    pthread_t thread1;
    pthread_create(&thread1, nullptr, &thread1_func, nullptr);

    pthread_t thread2;
    pthread_create(&thread2, nullptr, &thread2_func, nullptr);

    pthread_join(thread1, nullptr);
    pthread_join(thread2, nullptr);

    return 0;
}
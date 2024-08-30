//
// Created by baitianyu on 8/30/24.
//

#ifndef RPCFRAME_COROUTINE_HOOK_H
#define RPCFRAME_COROUTINE_HOOK_H

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dlfcn.h>

// const char*：指向常量字符的指针，指向的字符内容不可修改。指针所指向的字符是不可修改的。
// char *const：常量指针，指针本身的值不可修改，指向字符的内容可以修改。ptr 是一个常量指针，指向 arr，但不能指向其他内存地址。
// char * const cp; ( * 读成 pointer to )
// cp is a const pointer to char // 是一个const pointer，常量指针，不能修改为指向其他地址
// const char * p;
// p is a pointer to const char;  // 是一个pointer，其指向的字符串为const char，不能修改其指向的字符

// 1.使用using定义函数指针
// using func1 = int(*)(int, string);

// 2.使用typedef定义函数指针
// typedef  返回类型(*新类型)(参数表)
// 这里定义了read_fun_ptr_t类型，指向[返回类型(参数表)]这个函数指针
// 例如：typedef int(*func1)(int, string); // 和1中同种表达

// typedef int (PTypeFun1)(int, int); // 声明一个函数类型
// typedef int (*PTypeFun2)(int, int); // 声明一个函数指针类型
// int (*padd)(int, int); // 传统形式，定义一个函数指针变量
// PTypeFun1 *pTypeAdd1 = add;
// PTypeFun2 pTypeAdd2 = add;
// padd = add;

using read_fun_ptr_t = ssize_t(*)(int fd, void *buf, size_t count);
using write_fun_ptr_t = ssize_t(*)(int fd, const void *buf, size_t count);
using connect_fun_ptr_t = int (*)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
using accept_fun_ptr_t = int (*)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
using socket_fun_ptr_t = int (*)(int domain, int type, int protocol);
using sleep_fun_ptr_t = int (*)(unsigned int seconds);

namespace rocket {

    static bool g_hook = true;

    int accept_hook(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

    ssize_t read_hook(int fd, void *buf, size_t count);

    ssize_t write_hook(int fd, const void *buf, size_t count);

    int connect_hook(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    unsigned int sleep_hook(unsigned int seconds);

    void SetHook(bool val);

}

extern "C" {

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

ssize_t read(int fd, void *buf, size_t count);

ssize_t write(int fd, const void *buf, size_t count);

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

unsigned int sleep(unsigned int seconds);

}

#endif //RPCFRAME_COROUTINE_HOOK_H

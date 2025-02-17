//
// Created by baitianyu on 7/14/24.
//
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "common/util.h"

namespace mrpc {

    static int g_pid = 0;

    static thread_local int g_thread_id = 0;

    pid_t getPid() {
        if (g_pid != 0) {
            return g_pid;
        }
        return getpid();
    }

    pid_t getThreadId() {
        if (g_thread_id != 0) {
            return g_thread_id;
        }
        return syscall(SYS_gettid);
    }

    int64_t getNowMs() {
        timeval val;
        gettimeofday(&val, NULL);
        return val.tv_sec * 1000 + val.tv_usec / 1000;
    }

    int32_t getInt32FromNetByte(const char *buff) {
        int32_t ret;
        memcpy(&ret, buff, sizeof(ret));
        return ntohl(ret); // net to host long
    }

    std::string getLocalIP() {
        int sockfd;
        ifconf ifconf;
        ifreq *ifreq = nullptr;
        char buf[512];
        ifconf.ifc_len = 512;
        ifconf.ifc_buf = buf;
        // 这里使用完sockfd没有进行关闭，造成fd泄露
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            close(sockfd);
            return std::string{};
        }
        ioctl(sockfd, SIOCGIFCONF, &ifconf); // 设备所有接口
        ifreq = (struct ifreq *) ifconf.ifc_buf;
        for (int i = (ifconf.ifc_len / sizeof(ifreq)); i > 0; i--) {
            if (ifreq->ifr_flags == AF_INET) {
                // 找到ipv4的设备，并且为网卡，需要设置网卡名称
                if (ifreq->ifr_name == NETWORK_CARD_NAME) {
                    close(sockfd);
                    return std::string(inet_ntoa(((sockaddr_in *) &(ifreq->ifr_addr))->sin_addr));
                }
                ifreq++;
            }
        }
        close(sockfd);
        return std::string{};
    }
}
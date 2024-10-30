//
// Created by baitianyu on 8/12/24.
//

#ifndef RPCFRAME_HASH_BALANCE_H
#define RPCFRAME_HASH_BALANCE_H

#include <cstdint>
#include <map>
#include <set>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include "net/tcp/net_addr.h"

#define VIRTUAL_NODE_NUM 32

namespace rocket {
    class ConsistentHash {
    public:
        using con_hash_sptr_t_ = std::shared_ptr<ConsistentHash>;

        ConsistentHash() {};

        ~ConsistentHash() {
            serverNodes.clear();
        };

        // 192.168.1.1:22224，ip加端口的形式
        void AddNewPhysicalNode(const std::string &nodeIp, int virtualNodeNum);

        void DeletePhysicalNode(const std::string &nodeIp);

        std::string GetServerIP(const std::string &key);

        NetAddr::net_addr_sptr_t_ GetServer(const std::string &key);

        std::string printAllServerNodes();

    public:
        static NetAddr::net_addr_sptr_t_ toIPNetAddr(std::string &ip_and_port);

    private:
        // 只存储虚拟节点，key 是哈希值，value 是机器的 ip 地址
        std::map<uint32_t, std::string> serverNodes;
        // 每个机器节点关联的虚拟节点个数
        std::unordered_map<std::string, int> physicalServerAndVirtualNodeNum;
    };
}

#endif //RPCFRAME_HASH_BALANCE_H

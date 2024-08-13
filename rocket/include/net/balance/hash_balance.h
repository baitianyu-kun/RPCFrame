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
#include "net/tcp/net_addr.h"

#define VIRTUAL_NODE_NUM 32

namespace rocket {
    class ConsistentHash {
    public:
        using con_hash_sptr_t_ = std::shared_ptr<ConsistentHash>;

        explicit ConsistentHash(int virtualNodeNum) : virtualNodeNum(virtualNodeNum) {};

        ~ConsistentHash() {
            serverNodes.clear();
        };

        // 192.168.1.1:22224，ip加端口的形式
        void AddNewPhysicalNode(const std::string &nodeIp);

        void DeletePhysicalNode(const std::string &nodeIp);

        std::string GetServerIndex(const std::string &key);

        NetAddr::net_addr_sptr_t_ GetServer(const std::string &key);

        std::string printAllServerNodes();

    public:
        static NetAddr::net_addr_sptr_t_ toIPNetAddr(std::string &ip_and_port);

    private:
        // 真实+虚拟节点，key 是哈希值，value 是机器的 ip 地址
        std::map<uint32_t, std::string> serverNodes;
        // 每个机器节点关联的虚拟节点个数
        int virtualNodeNum;
        // 起到看该物理ip有没有被插入的作用，防止serverNode中重复插入
        std::unordered_map<std::string, uint32_t> physicalServerNodeCounts;
    };
}

#endif //RPCFRAME_HASH_BALANCE_H

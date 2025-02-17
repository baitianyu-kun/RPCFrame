//
// Created by baitianyu on 2/11/25.
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

#define VIRTUAL_NODE_NUM Config::GetGlobalConfig()->m_virtual_node_num

namespace mrpc {
    class ConsistentHash {
    public:
        using ptr = std::shared_ptr<ConsistentHash>;

        ConsistentHash() {};

        ~ConsistentHash() {
            serverNodes.clear();
        };

        // 192.168.1.1:22224，ip加端口的形式
        void addNewPhysicalNode(const std::string &nodeIp, int virtualNodeNum);

        void deletePhysicalNode(const std::string &nodeIp);

        std::string getServerIP(const std::string &key);

        NetAddr::ptr getServer(const std::string &key);

        std::string printAllServerNodes();

    public:
        static NetAddr::ptr toIPNetAddr(std::string &ip_and_port);

    private:
        // 只存储虚拟节点，key 是哈希值，value 是机器的 ip 地址
        std::map<uint32_t, std::string> serverNodes;
        // 每个机器节点关联的虚拟节点个数
        std::unordered_map<std::string, int> physicalServerAndVirtualNodeNum;
    };
}

#endif //RPCFRAME_HASH_BALANCE_H

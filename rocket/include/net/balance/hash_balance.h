//
// Created by baitianyu on 8/12/24.
//

#ifndef RPCFRAME_HASH_BALANCE_H
#define RPCFRAME_HASH_BALANCE_H

#include <cstdint>
#include <map>
#include <set>
#include <memory>

namespace rocket {
    class ConsistentHash {
    public:
        using con_hash_sptr_t_ = std::shared_ptr<ConsistentHash>;

        explicit ConsistentHash(int virtualNodeNum) : virtualNodeNum(virtualNodeNum) {};

        ~ConsistentHash() {
            serverNodes.clear();
        };

        void Initialize();

        void AddNewPhysicalNode(const std::string &nodeIp);

        void DeletePhysicalNode(const std::string &nodeIp);

        std::string GetServerIndex(const std::string &key);

    private:
        std::map<uint32_t, std::string> serverNodes;
        // 虚拟节点，key 是哈希值，value 是机器的 ip 地址
        std::set<std::string> physicalNodes;  // 真实机器节点 ip
        int virtualNodeNum;  // 每个机器节点关联的虚拟节点个数
    };
}

#endif //RPCFRAME_HASH_BALANCE_H

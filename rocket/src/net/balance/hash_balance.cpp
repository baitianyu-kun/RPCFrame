//
// Created by baitianyu on 8/12/24.
//
#include <sstream>
#include "net/balance/hash_balance.h"
#include "common/log.h"
#include "common/encode_util.h"

namespace rocket {
    void rocket::ConsistentHash::Initialize() {
        for (auto &ip: physicalNodes) {
            for (int j = 0; j < virtualNodeNum; ++j) {
                std::stringstream nodeKey;
                nodeKey << ip << "#" << j;
                uint32_t partition = murmur3_32(nodeKey.str().c_str(), nodeKey.str().size());
                serverNodes.insert({partition, ip});
            }
        }
    }

    void ConsistentHash::AddNewPhysicalNode(const std::string &nodeIp) {
        for (int j = 0; j < virtualNodeNum; ++j) {
            std::stringstream nodeKey;
            nodeKey << nodeIp << "#" << j;
            uint32_t partition = murmur3_32(nodeKey.str().c_str(), nodeKey.str().size());
            serverNodes.insert({partition, nodeIp});
        }
    }

    void ConsistentHash::DeletePhysicalNode(const std::string &nodeIp) {
        for (int j = 0; j < virtualNodeNum; ++j) {
            std::stringstream nodeKey;
            nodeKey << nodeIp << "#" << j;
            uint32_t partition = murmur3_32(nodeKey.str().c_str(), nodeKey.str().size());
            auto it = serverNodes.find(partition);
            if (it != serverNodes.end()) {
                serverNodes.erase(it);
            }
        }
    }

    std::string ConsistentHash::GetServerIndex(const std::string &key) {
        uint32_t partition = murmur3_32(key.c_str(), key.size());
        auto it = serverNodes.lower_bound(partition);
        // 沿环的顺时针找到一个大于等于 partition 的虚拟节点
        if (it == serverNodes.end()) {
            if (serverNodes.empty()) {
                ERRORLOG("no available nodes");
            }
            return serverNodes.begin()->second;
        }
        return it->second;
    }
}




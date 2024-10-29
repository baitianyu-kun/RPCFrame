//
// Created by baitianyu on 8/12/24.
//
#include <sstream>
#include "net/balance/hash_balance.h"
#include "common/log.h"
#include "common/encode_util.h"

namespace rocket {

    void ConsistentHash::AddNewPhysicalNode(const std::string &nodeIp) {
        if (physicalServerNodeCounts.find(nodeIp) == physicalServerNodeCounts.end()) {
            for (int j = 0; j < virtualNodeNum; ++j) {
                std::stringstream nodeKey;
                // 实际上只把虚拟节点加入到哈希环上了
                nodeKey << nodeIp << "#" << j;
                uint32_t partition = murmur3_32(nodeKey.str().c_str(), nodeKey.str().size());
                serverNodes.insert({partition, nodeIp});
                physicalServerNodeCounts[nodeIp]++;
            }
        } else {
            // 没插入过才进行插入
            return;
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
                physicalServerNodeCounts.erase(nodeIp);
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

    NetAddr::net_addr_sptr_t_ ConsistentHash::toIPNetAddr(std::string &ip_and_port) {
        auto find_res = ip_and_port.find(':');
        std::string ip, port;
        if (find_res != ip_and_port.npos) {
            ip = ip_and_port.substr(0, find_res);
            port = ip_and_port.substr(find_res + 1, ip_and_port.length() - find_res);
        }
        return std::make_shared<IPNetAddr>(ip, std::stoi(port));
    }

    NetAddr::net_addr_sptr_t_ ConsistentHash::GetServer(const std::string &key) {
        auto idx = GetServerIndex(key);
        DEBUGLOG("Hash Balance to server: %s", idx.c_str());
        return toIPNetAddr(idx);
    }

    std::string ConsistentHash::printAllServerNodes() {
        std::string all_server_node = "";
        for (const auto &item: serverNodes) {
            all_server_node = all_server_node + item.second + ",";
        }
        return all_server_node.substr(0, all_server_node.length() - 1);
    }
}




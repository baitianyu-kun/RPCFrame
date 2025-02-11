//
// Created by baitianyu on 2/11/25.
//
#include <sstream>
#include "net/balance/hash_balance.h"
#include "common/log.h"
#include "common/encode_util.h"

namespace rocket {
    void ConsistentHash::addNewPhysicalNode(const std::string &nodeIp, int virtualNodeNum) {
        if (physicalServerAndVirtualNodeNum.find(nodeIp) == physicalServerAndVirtualNodeNum.end()) {
            physicalServerAndVirtualNodeNum.emplace(nodeIp, virtualNodeNum);
            for (int j = 0; j < virtualNodeNum; ++j) {
                std::stringstream nodeKey;
                // 实际上只把虚拟节点加入到哈希环上了
                nodeKey << nodeIp << "#" << j;
                uint32_t partition = murmur3_32(nodeKey.str().c_str(), nodeKey.str().size());
                serverNodes.insert({partition, nodeIp});
            }
        } else {
            // 没插入过才进行插入
            return;
        }
    }

    void ConsistentHash::deletePhysicalNode(const std::string &nodeIp) {
        auto server_and_vnode_num = physicalServerAndVirtualNodeNum.find(nodeIp);
        if (server_and_vnode_num != physicalServerAndVirtualNodeNum.end()) {
            for (int j = 0; j < server_and_vnode_num->second; ++j) {
                std::stringstream nodeKey;
                nodeKey << nodeIp << "#" << j;
                uint32_t partition = murmur3_32(nodeKey.str().c_str(), nodeKey.str().size());
                auto it = serverNodes.find(partition);
                if (it != serverNodes.end()) {
                    serverNodes.erase(it);
                }
            }
            physicalServerAndVirtualNodeNum.erase(nodeIp);
        } else {
            return;
        }
    }

    std::string ConsistentHash::getServerIP(const std::string &key) {
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

    NetAddr::ptr ConsistentHash::toIPNetAddr(std::string &ip_and_port) {
        auto find_res = ip_and_port.find(':');
        std::string ip, port;
        if (find_res != ip_and_port.npos) {
            ip = ip_and_port.substr(0, find_res);
            port = ip_and_port.substr(find_res + 1, ip_and_port.length() - find_res);
        }
        return std::make_shared<IPNetAddr>(ip, std::stoi(port));
    }

    NetAddr::ptr ConsistentHash::getServer(const std::string &key) {
        auto ip = getServerIP(key);
        DEBUGLOG("Hash Balance to server: %s", ip.c_str());
        return toIPNetAddr(ip);
    }

    std::string ConsistentHash::printAllServerNodes() {
        std::string all_server_node = "";
        for (const auto &item: serverNodes) {
            all_server_node = all_server_node + item.second + ",";
        }
        return all_server_node.substr(0, all_server_node.length() - 1);
    }
}
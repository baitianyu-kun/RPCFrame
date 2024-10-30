//
// Created by baitianyu on 8/12/24.
//
#include <sstream>
#include "net/balance/hash_balance.h"
#include "common/log.h"
#include "common/encode_util.h"

namespace rocket {

    void ConsistentHash::AddNewPhysicalNode(const std::string &nodeIp, int virtualNodeNum) {
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

    void ConsistentHash::DeletePhysicalNode(const std::string &nodeIp) {
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

    std::string ConsistentHash::GetServerIP(const std::string &key) {
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
        auto ip = GetServerIP(key);
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

// 测试负载量
//    auto hash = make_shared<ConsistentHash>(50);
//    hash->AddNewPhysicalNode("192.168.1.1:8080");
//    hash->AddNewPhysicalNode("192.168.1.2:8080");
//    hash->AddNewPhysicalNode("192.168.1.3:8080");
//    hash->AddNewPhysicalNode("192.168.1.4:8080");
//    unordered_map<string, int> maps; // 记录每个地址的访问量
//
//    srand((unsigned) time(NULL));
//    int a = 0, b = 192;
//    int fangwen_cishu = 5000000;
//    for (int i = 0; i < fangwen_cishu; ++i) {
//        auto rand1 = (rand() % (b - a + 1)) + a;
//        auto rand2 = (rand() % (b - a + 1)) + a;
//        auto rand3 = (rand() % (b - a + 1)) + a;
//        auto rand4 = (rand() % (b - a + 1)) + a;
//        string addr =
//                to_string(rand1) + "." + to_string(rand2) + "." + to_string(rand3) + "." + to_string(rand4) + ":80";
//        maps[hash->GetServerIndex(addr)]++;
//    }
//
//    for (const auto &item: maps) {
//        double rate = ((1.0 * item.second) / fangwen_cishu) * 100;
//        cout << "Node: " << item.first << "        Rate:" << rate << endl;
//    }
}




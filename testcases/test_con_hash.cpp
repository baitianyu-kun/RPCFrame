//
// Created by baitianyu on 24-10-30.
//
#include "bits/stdc++.h"
#include "net/balance/hash_balance.h"

using namespace std;

int main() {
    // 测试负载量
    auto hash = make_shared<rocket::ConsistentHash>();
    int virtual_node = 160;
    int double_virtual_node = 320;
    hash->AddNewPhysicalNode("192.168.1.1:8080", virtual_node);
    hash->AddNewPhysicalNode("192.168.1.2:8080", virtual_node);
    hash->AddNewPhysicalNode("192.168.1.3:8080", double_virtual_node);
    hash->AddNewPhysicalNode("192.168.1.4:8080", virtual_node);
    unordered_map<string, int> maps; // 记录每个地址的访问量

    srand((unsigned) time(nullptr));

    int a = 0, b = 192;
    int fangwen_cishu = 5000000;
    for (int i = 0; i < fangwen_cishu; ++i) {
        auto rand1 = (rand() % (b - a + 1)) + a;
        auto rand2 = (rand() % (b - a + 1)) + a;
        auto rand3 = (rand() % (b - a + 1)) + a;
        auto rand4 = (rand() % (b - a + 1)) + a;
        string addr =
                to_string(rand1) + "." + to_string(rand2) + "." + to_string(rand3) + "." + to_string(rand4) + ":80";
        maps[hash->GetServerIP(addr)]++;
    }

    for (const auto &item: maps) {
        double rate = ((1.0 * item.second) / fangwen_cishu) * 100;
        cout << "Node: " << item.first << "        Rate:" << rate << endl;
    }
}
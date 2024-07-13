//
// Created by baitianyu on 7/12/24.
//
#include <iostream>

using namespace std;

void Print() {}

template<typename T, typename ... Args>
void Print(T first, Args &&...args) {
    cout << "arg: " << first << endl;
    Print(args...);
}

int main() {
    Print(1);
    Print(1, "ab");
    Print(1.0, "23", 4);
    return 0;
}

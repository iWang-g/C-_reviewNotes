#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>

std::vector<int> data = {1, 2, 3};
std::shared_mutex rwMutex;
std::mutex print;

/*
g++ .\share_mutex.cpp -std=c++17
锁类型：
    shared_lock：多个读取者共享访问
    unique_lock：单个写入者独占访问
适用场景：
    读多写少的数据结构
    配置信息、缓存系统
*/
void reader(int id) {
    std::shared_lock lock(rwMutex); // 共享锁（多读）
    std::lock_guard<std::mutex> lock_print(print);
    std::cout << "Reader " << id << " sees: ";
    for (int n : data) std::cout << n << " ";
    std::cout << "\n";
}

void writer() {
    std::unique_lock lock(rwMutex); // 独占锁（单写）
    data.push_back(data.back() + 1);
}

int main() {
    std::thread readers[4];
    for (int i = 0; i < 4; ++i)
        readers[i] = std::thread(reader, i);
    
    std::thread writerThread(writer);
    
    for (auto& t : readers) t.join();
    writerThread.join();

    return 0;
}
#include <iostream>
#include <semaphore>
#include <thread>

std::counting_semaphore<10> sem(3); // 最大计数10，初始3

void worker(int id) {
    sem.acquire(); // P操作（获取信号量）
    std::cout << "Thread " << id << " working...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sem.release(); // V操作（释放信号量）
}

int main() {
    std::thread threads[5];
    for (int i = 0; i < 5; ++i)
        threads[i] = std::thread(worker, i);
    
    for (auto& t : threads) t.join();
    return 0;
}
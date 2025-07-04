#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
using namespace std;

queue<int> data_queue;
mutex queue_mutex;
condition_variable data_cond;
constexpr int MAX_SIZE = 10;

void producer(int id) {
    for (int i = 0; i < 5; ++i) {
        this_thread::sleep_for(chrono::milliseconds(100));

        unique_lock<mutex> lock(queue_mutex);
        data_cond.wait(lock, []{
            return data_queue.size() < MAX_SIZE;
        });

        data_queue.push(i);
        std::cout << "Producer " << id << " produced " << i << std::endl;
        lock.unlock();
        data_cond.notify_all();
    }
}

void consumer(int id) {
    while(true) {
        unique_lock<mutex> lock(queue_mutex);
        data_cond.wait(lock, []{
            return !data_queue.empty();
        });

        int val = data_queue.front();
        data_queue.pop();
        std::cout << "Consumer " << id << " consumed " << val << std::endl;
        lock.unlock();
        data_cond.notify_all();

        if(val == 4) break;
    }
}

int main()
{
    thread producers[2], consumers[2];

    for (int i = 0; i < 2; ++i) {
        producers[i] = thread(producer, i);
        consumers[i] = thread(consumer, i);
    }

    for (auto &t : producers) t.join();
    for (auto &t : consumers) t.join();

    return 0;
}
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
constexpr int MAX_SIZE = 10; // 队列最大容量

void producer(int id) {
    // 每个生产者线程循环生成5个数据
    for (int i = 0; i < 5; ++i) {
        this_thread::sleep_for(chrono::milliseconds(100)); // 模拟数据生成耗时

        unique_lock<mutex> lock(queue_mutex);
        /*
            等待条件：data_cond.wait(lock, 条件) 做两件事：
                先检查条件（data_queue.size() < MAX_SIZE），如果满足则继续；
                如果不满足（队列已满），则解锁互斥锁并阻塞线程，等待被唤醒（消费者取走数据后会唤醒）。
            虚假唤醒：条件变量在没有收到通知的情况下也可能返回。
            因此：1.总是使用带有谓词条件的wait版本；2.谓词应该检查实际业务条件，而不仅仅是标志位。
        */
        data_cond.wait(lock, []{
            return data_queue.size() < MAX_SIZE; // lambda的返回值：bool类型，返回给wait函数，用于决定线程是否继续等待
        });

        // 队列不满时，生产者将数据 i 放入队列，并打印生产信息
        data_queue.push(i);
        std::cout << "Producer " << id << " produced " << i << std::endl;
        lock.unlock(); // 显式解锁（可选，unique_lock 析构时也会解锁，这里提前释放以提高并发）
        data_cond.notify_all(); // 唤醒所有等待的线程（消费者可能在等待队列非空）
    }
}

void consumer(int id) {
    while(true) {
        unique_lock<mutex> lock(queue_mutex);
        /*
            如果队列有数据（!empty()），则继续；
            如果队列空，则解锁互斥锁并阻塞线程，等待生产者放入数据后唤醒。
        */
        data_cond.wait(lock, []{
            return !data_queue.empty();
        });

        // 队列非空时，消费者取出数据 val 并打印消费信息
        int val = data_queue.front();
        data_queue.pop();
        std::cout << "Consumer " << id << " consumed " << val << std::endl;
        lock.unlock();
        data_cond.notify_all(); // 唤醒所有等待的线程（生产者可能在等队列不满）

        if(val == 4) break; // 当消费到 val=4 时，退出循环（结束线程）
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
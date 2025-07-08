#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

/*
    屏障（barrier）是多线程同步原语，用于让一组线程在某个“checkpoint（检查点）”
    处等待，直到所有线程都到达该点，再一起继续执行。
    例如，代码中的 arrive_and_wait 就是一个检查点：
        所有线程都执行完Phase 1后，才能进入Pharse 2；
        所有线程都执行完Phase 2后，才能结束（或进入下一个阶段）。
*/
/*无法使用C++20，可以用 std::condition_variable+std::mutex 手动实现简化barrier*/
class Barrier {
private:
    std::mutex mtx;             // 互斥锁，保护共享变量
    std::condition_variable cv; // 条件变量，用于线程等待/唤醒
    int expected;               // 屏障需要等待的总线程数（如代码中的3）
    int arrived;                // 当前已到达屏障的线程数（初始为0）
    int phase;                  // 当前同步阶段（避免“虚假唤醒”，关键！）

public:
    explicit Barrier(int count) : expected(count), arrived(0), phase(0) {}

    void arrive_and_wait() {
        // 1. 加锁；保护共享变量
        std::unique_lock<std::mutex> lock(mtx);

        // 2. 记录当前阶段（避免虚假唤醒的关键）
        int current_phase = phase;

        // 3. 已到达线程数 +1
        arrived++;

        // 4. 判断是否所有线程都到达
        if (arrived == expected) {
            arrived = 0;     // 重置计数，为下一个阶段做准备
            phase++;         // 推进阶段
            cv.notify_all(); // 唤醒所有等待的线程
        } else {
            // 5. 等待，直到当前阶段结束
            cv.wait(lock, [this, current_phase](){
                return current_phase != phase; // 条件：阶段已变化
            });
        }
    }
};

// 使用方式与std::barrier一致
Barrier syncPoint(3);

void task(int id) {
    std::cout << "Phase 1 - Thread " << id << "\n";
    syncPoint.arrive_and_wait();

    std::cout << "Phase 2 - Thread " << id << "\n";
    syncPoint.arrive_and_wait();
}

int main() {
    std::thread t1(task, 1);
    std::thread t2(task, 2);
    std::thread t3(task, 3);

    t1.join();
    t2.join();
    t3.join();

    return 0;
}
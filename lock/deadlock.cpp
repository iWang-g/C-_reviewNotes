#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

std::mutex t_lock1;
std::mutex t_lock2;
int m_1 = 0;
int m_2 = 1;

void dead_lock1() {
    while (true) {
        std::cout << "dead_lock1 begin " << std::endl;
        t_lock1.lock();
        m_1 = 1024;
        t_lock2.lock();
        m_2 = 2048;
        t_lock2.unlock();
        t_lock1.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::cout << "dead_lock2 end " << std::endl;
    }
}

void dead_lock2() {
    while (true) {
        std::cout << "dead_lock2 begin " << std::endl;
        t_lock2.lock();
        m_2 = 2048;
        t_lock1.lock();
        m_1 = 1024;
        t_lock1.unlock();
        t_lock2.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::cout << "dead_lock2 end " << std::endl;
    }
}

void test_dead_lock() {
    std::thread t1(dead_lock1);
    std::thread t2(dead_lock2);
    t1.join();
    t2.join();
}

/*
    避免死锁的一个方式就是将加锁和解锁的功能封装为独立的函数，
    这样能保证在独立的函数里执行完操作后就解锁，不会导致一个函数里使用多个锁的情况
*/
// 加锁和解锁作为原子操作解耦合，各自只管理自己的功能
void atomic_lock1() {
    std::cout << "lock1 begin lock" << std::endl;
    t_lock1.lock();
    m_1 = 1024;
    t_lock1.unlock();
    std::cout << "lock1 end lock" << std::endl;
}

void atomic_lock2() {
    std::cout << "lock2 begin lock" << std::endl;
    t_lock2.lock();
    m_2 = 2048;
    t_lock2.unlock();
    std::cout << "lock2 end lock" << std::endl;
}

void safe_lock1() {
    while(true) {
        atomic_lock1();
        atomic_lock2();
        this_thread::sleep_for(1s);
    }
}

void safe_lock2() {
    while(true) {
        atomic_lock2();
        atomic_lock1();
        this_thread::sleep_for(1s);
    }
}

void test_safe_lock() {
    thread t1(safe_lock1);
    thread t2(safe_lock2);
    t1.join();
    t2.join();
}

int main()
{
    // test_dead_lock();
    test_safe_lock();
    
    return 0;
}
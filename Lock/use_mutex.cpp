#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

mutex _mutex;
int share_data = 100;

void use_lock()
{
    while(true) {
        _mutex.lock();
        share_data++;
        cout << "current thread id is " << this_thread::get_id() << endl;
        cout << "share_data:" << share_data << endl;
        _mutex.unlock();
        this_thread::sleep_for(1s);
    }
}

void test_lock()
{
    thread t1(use_lock);

    thread t2([](){
        while(true) {
            /*
                lock_guard在作用域结束时自动调用其析构函数解锁，这么做的一个好处是
                简化了一些特殊情况从函数中返回的写法，比如异常或者条件不满足时，函数内部直接return，锁也会自动解开
            */
            lock_guard<mutex> lock(_mutex); // 自动加锁和解锁
            share_data--;
            cout << "current thread id is " << this_thread::get_id() << endl;
            cout << "share_data:" << share_data << endl;
            this_thread::sleep_for(1s);
    }
    });

    t1.join();
    t2.join();
}

int main()
{
    test_lock();

    return 0;
}
#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

void task()
{
    cout << "线程正在执行任务..." << endl;

    // 让当前线程休眠2秒
    this_thread::sleep_for(chrono::seconds(2));

    cout << "线程休眠结束" << endl;
}

int main()
{
    thread t(task); // 创建线程
    t.join(); // 等待线程结束

    return 0;
}
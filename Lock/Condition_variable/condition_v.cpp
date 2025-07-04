#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

mutex mtx;
condition_variable cv;
bool ready = false;

/*
    lock_guard vs unique_lock:
    lock_guard更加轻量级，少了一些成员函数；
    unique_lock类有unlock函数，可以手动释放锁，所以条件变量都配合unique_lock
    使用（条件变量在wait时需要有手动释放锁的能力）
*/
void worker_thread() {
    // 等待主线程发送通知
    unique_lock<mutex> lock(mtx);
    cout << "工作线程进入等待状态" << endl;
    cv.wait(lock, []{
        return ready;
    });
    cout << "接收到主线程发送的通知" << endl;
    cout << "Worker thread is processing..." << std::endl;
}

/*
    1. 工作线程调用cv.wait()进入等待状态
    2. 主线程在准备工作完成后设置ready=true并调用cv.notify_one()
    3. 工作线程被唤醒，检查条件ready为真后继续执行
*/
void test_worker_thread() {
    thread worker(worker_thread);

    // 模拟准备工作
    cout << "readying..." << std::endl;
    this_thread::sleep_for(chrono::seconds(1));
    cout << "ready finished" << std::endl;

    {
       lock_guard<mutex> lock(mtx);
       ready = true;
       cv.notify_one(); // 通知等待的线程
    }

    worker.join();
}

int main()
{
    test_worker_thread();
    return 0;
}
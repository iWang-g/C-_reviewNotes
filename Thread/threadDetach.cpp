#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

struct func
{
    int m_i;
    
    func(int i) : m_i(i) {}

    void operator()() {
        for(int i = 0; i < 3; i++) {
            m_i = i;
            cout << "m_i:" << m_i << endl;
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
};

void oops()
{
    int val = 0;
    func myfunc(val);
    thread t(myfunc);

    // detach: 将子线程与主线程分离，子线程在后台独立运行，主线程不再等待它
    t.detach();
}

int main()
{
    oops();

    cout << "main thread" << endl;
    // 防止主线程退出过快，需要停顿一下，让子线程跑起来detach
    this_thread::sleep_for(chrono::seconds(3));
    return 0;
}
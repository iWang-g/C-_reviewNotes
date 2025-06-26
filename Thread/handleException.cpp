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

// 代码臃肿，缺陷很多
void catch_exception()
{
    int val = 0;
    func myfunc(val);

    thread t(myfunc);
    try {
        //本线程做一些事情,可能引发崩溃
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch(exception& e) {
        t.join();
        throw;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// RAII模式管理线程
class thread_guard
{
    thread& m_t;
public:
    explicit thread_guard(thread& t) : m_t(t) {}

    ~thread_guard() {
        if(m_t.joinable())
            m_t.join(); // 自动等待线程结束
    }

    // 禁止拷贝，避免多个管理者竞争释放资源
    thread_guard(const thread_guard&) = delete;
    thread_guard& operator=(const thread_guard&) = delete;
};

void catch_exception_safe()
{
    int val;
    func myfunc(val);

    thread t(myfunc);
    thread_guard g(t); // RAII守卫确保线程终止

    try {
        // 可能抛出异常的操作
        this_thread::sleep_for(1s);
    } catch(...) { // 捕获所有异常类型
        throw; // 统一重新抛出
    }
}

int main()
{
    catch_exception_safe();

    return 0;
}
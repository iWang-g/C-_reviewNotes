#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
using namespace std;

void some_function()
{
    cout << "some_function" << endl;
    this_thread::sleep_for(1s);
}

void some_other_function()
{
    cout << "some_other_function" << endl;
    this_thread::sleep_for(1s);
}

void bind_thread()
{
    // t1 绑定 some_function
    thread t1(some_function);
    // 转移 t1 管理的线程给 t2，转移后 t1 无效
    thread t2 = move(t1);
    // t1 也可以绑定其他线程，执行 some_other_function
    t1 = thread(some_other_function);
    // 创建一个线程变量 t3
    thread t3;
    // 转移 t2 管理的线程给 t3
    t3 = move(t2);
    // 转移t3管理的线程给t1
    /*
        这一步会导致程序崩溃，此时 t1 正在管理运行 some_other_function，
        所以不要将一个线程的管理权交给一个已绑定线程的变量，否则会触发线程的 terminate 函数引发崩溃
    */
    t1 = std::move(t3);
    std::this_thread::sleep_for(std::chrono::seconds(2000));
}

/*
    可以在函数内部返回一个局部的 std::thread 对象
    thread 删除了拷贝构造，但提供了移动构造，编译器
    自动调用移动构造，将线程的所有权转移给新对象
*/
thread f()
{
    return thread(some_function);
}

void fun()
{
    thread t = f();
    t.join();
}

// RAII守卫确保异常时清理
struct ThreadGuard {
    std::vector<std::thread>& threads;
    ~ThreadGuard() {
        for(auto& t : threads) 
            if(t.joinable()) t.join();
    }
};

mutex _mutex;
void param_function(int val)
{
    _mutex.lock();
    cout << "val:" << val << endl;
    _mutex.unlock();
    this_thread::sleep_for(1s);
}

void use_vector()
{
    vector<thread> threads;
    ThreadGuard guard{threads};

    for(int i = 0; i < 10; i++) {
        threads.emplace_back(param_function, i);
    }
}

/*
    RAII风格的线程包装器，确保线程在销毁时会自动join（如果可连接）。
    同时，它支持移动语义，禁止拷贝（因为线程资源是独占的）
*/
class joining_thread
{
    thread _t;
public:
    // 创建空线程对象，默认构造状态安全
    joining_thread() noexcept = default;

    // 显示构造函数：接管已有线程，移动语义转移所有权
    explicit joining_thread(thread t) noexcept : _t(move(t)) {
        /* 移动后原t变为空状态 */
    }

    // 通用构造函数：支持任意可调用对象和参数（完美转发）
    template<typename Callable, typename ...Args>
    explicit joining_thread(Callable&& func, Args&& ...args):
        _t(forward<Callable>(func), forward<Args>(args)...) 
    {
        /* 在构造时启动线程 */
    }

    // 移动构造：资源所有权转移
    joining_thread(joining_thread&& other) noexcept : _t(move(other._t)) {
        /* 移动后原对象不再管理线程 */
    }

    // 移动赋值：先清理当前资源，再接管新资源
    joining_thread& operator=(joining_thread&& other) noexcept
    {
        if(this != &other) {
            // 如果当前线程可汇合，则汇合等待线程完成再赋值
            if(joinable()) {
                join(); // 关键：确保当前线程安全退出
            }

            _t = move(other._t); // 资源转移
        }
        return *this;
    }

    // 析构函数（RAII核心）：自动等待线程结束
    ~joining_thread() {
        if(joinable()) { // 若线程可连接则阻塞等待
            join();
        }
    }

    // 线程控制接口（委托给内部_t）
    void swap(joining_thread& other) noexcept {
        _t.swap(other._t);
    }

    // 检查一个线程是否可以加入，用于判断线程是否调用过join或detach，若没有调用过join或detach，返回true
    bool joinable() const noexcept {
        return _t.joinable();
    }

    void join() {
        _t.join();
    }

    void detach() {
        _t.detach();
    }

    thread::id get_id() const noexcept {
        return _t.get_id();
    }

    // 禁用拷贝（线程资源不可共享）
    joining_thread(const joining_thread&) = delete;
    joining_thread& operator=(const joining_thread&) = delete;
};

// 使用场景
void safe_concurrency()
{
    vector<joining_thread> threads;

    for(int i = 0; i < 5; i++)
    {
        threads.emplace_back([](int id){
            lock_guard<mutex> lock(_mutex); // 自动加锁和解锁
            cout << "Thread" << id << "working" << endl;
        }, i);
    }
    // 析构时自动等待所有线程结束（无需显式join）
}// RAII在此触发所有线程同步

int main()
{
    safe_concurrency();

    return 0;
}
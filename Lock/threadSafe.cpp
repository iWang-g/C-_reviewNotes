#include <iostream>
#include <mutex>
#include <stack>
#include <memory>
#include <exception>
#include <vector>
#include <thread>
#include <assert.h>
#include <atomic>
using namespace std;

// 线程安全的栈模板类
template<typename T>
class threadsafe_stack {
private:
    std::stack<T> data;   // 底层存储数据的栈
    mutable std::mutex mtx; // 互斥锁（mutable允许在const方法中加锁）

public:
    threadsafe_stack() = default;

    // 拷贝构造函数：锁定源对象的互斥锁后复制数据
    threadsafe_stack(const threadsafe_stack& other) {
        std::lock_guard<std::mutex> lock(other.mtx);
        data = other.data; // 复制整个栈（注意性能开销）
    }

    threadsafe_stack& operator=(const threadsafe_stack&) = delete; // 禁用赋值操作

    // 压栈操作（线程安全）
    void push(T new_value) {
        std::lock_guard<std::mutex> lock(mtx);
        data.push(std::move(new_value)); // 使用move避免拷贝
    }

    // 弹栈操作（返回智能指针，异常安全）
    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (data.empty()) throw std::out_of_range("Stack is empty!"); // 空栈检查
        auto res = std::make_shared<T>(std::move(data.top())); // 移动构造减少拷贝
        data.pop(); // 确保构造成功后再弹出
        return res;
    }

    // 弹栈操作（通过引用返回结果）
    void pop(T& value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (data.empty()) throw std::out_of_range("Stack is empty!");
        value = std::move(data.top()); // 移动赋值
        data.pop();
    }

    // 检查栈是否为空（线程安全）
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.empty();
    }

    // 返回栈大小（线程安全）
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.size();
    }
};

void test_threadsafe_stack() {
    threadsafe_stack<int> safe_stack;
    safe_stack.push(1);
    // safe_stack.push(2);
    // safe_stack.push(3);

    thread t1([&safe_stack](){
        if(!safe_stack.empty()) {
            cout << "current thread id is " << this_thread::get_id() << endl;
            this_thread::sleep_for(1s);
            safe_stack.pop();
        }
    });

    thread t2([&safe_stack](){
        if(!safe_stack.empty()) {
            cout << "current thread id is " << this_thread::get_id() << endl;
            this_thread::sleep_for(1s);
            safe_stack.pop();
        }
    });

    t1.join();
    t2.join();
}

// 基本功能测试（单线程）
void test_basic_operations() {
    threadsafe_stack<int> stack;
    
    // 测试压栈
    stack.push(1);
    stack.push(2);
    assert(stack.size() == 2);

    // 测试弹栈（智能指针版本）
    auto top1 = stack.pop();
    assert(*top1 == 2);
    assert(stack.size() == 1);

    // 测试弹栈（引用版本）
    int value;
    stack.pop(value);
    assert(value == 1);
    assert(stack.empty());

    std::cout << "Basic operations test passed.\n";
}

// 异常安全测试
void test_exception_safety() {
    threadsafe_stack<std::vector<int>> stack;
    
    // 构造一个可能抛出异常的大对象
    std::vector<int> large_vec(1000000, 42);
    stack.push(large_vec);

    try {
        auto top = stack.pop(); // 拷贝可能因内存不足抛出异常
    } catch (const std::bad_alloc& e) {
        // 验证异常后栈状态未变
        assert(!stack.empty());
        std::cout << "Exception caught: " << e.what() << "\n";
    }

    std::cout << "Exception safety test passed.\n";
}

// 多线程并发测试
void test_concurrent_access() {
    threadsafe_stack<int> stack;
    const int kNumThreads = 4;
    const int kPushesPerThread = 1000;
    std::vector<std::thread> threads;

    // 启动生产者线程（并发压栈）
    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < kPushesPerThread; ++j) {
                stack.push(j);
                // 添加线程日志
                // static std::mutex print_mutex;
                // {
                //     std::lock_guard<std::mutex> lock(print_mutex);
                //     std::cout << "Thread " << std::this_thread::get_id() 
                //             << " pushed value: " << j << std::endl;
                //     this_thread::sleep_for(1s);
                // }
            }
        });
    }

    // 启动消费者线程（并发弹栈）
    std::atomic<int> pop_count(0);
    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([&] {
            while (pop_count < kNumThreads * kPushesPerThread) {
                try {
                    stack.pop();
                    ++pop_count;
                } catch (const std::out_of_range&) {
                    std::this_thread::yield(); // 避免忙等待
                }
            }
        });
    }

    for (auto& t : threads) t.join();
    
    // 验证最终栈为空
    assert(stack.empty());
    assert(pop_count == kNumThreads * kPushesPerThread);
    std::cout << "Concurrent access test passed.\n";
}

int main()
{
    // test_threadsafe_stack();
    // test_basic_operations();
    // test_exception_safety();
    test_concurrent_access();

    return 0;
}
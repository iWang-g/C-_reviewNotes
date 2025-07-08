#include <iostream>
#include <future>
#include <thread>

int compute(int a, int b) {
    return a * b; 
}

void test_compute() {
    // future的模板参数只依赖返回类型，compute返回int
    std::future<int> fut = std::async(std::launch::async, compute, 2, 3);

    // 主线程继续工作
    std::cout << "Main thread working..." << std::endl;

    // 获取结果（阻塞直到完成）
    int result = fut.get();
    std::cout << "Result: " << result << std::endl;
}

void test_packaged_task() {
    // 包装任务（签名: int()，无参数，返回int）
    std::packaged_task<int()> task1([]{
        return 2 + 3;
    });

    std::packaged_task<int(int, int)> task2([](int a, int b){
        return a + b;
    });

    // 获取关联的 future
    std::future<int> fut = task2.get_future();

    // 在独立线程中执行任务
    std::thread t(std::move(task2), 2, 4);
    t.detach();

    std::cout << "Result: " << fut.get() << std::endl;
}

// 异步除法函数：接收 promise 对象和两个整数
void asyncDivision(std::promise<double>&& prom, int a, int b) {
    if(b == 0) {
        // 除数为零时设置异常
        prom.set_exception(
            std::make_exception_ptr(
                std::runtime_error("Div by zero")
            )
        );
    } else {
        // 计算结果并存入 promise
        prom.set_value(static_cast<double>(a) / b);
    }
}

// 测试函数
void test_asyncDivision() {
    // 创建 promise-future
    std::promise<double> prom;               // 生产者承诺提供 double 结果
    std::future<double> fut = prom.get_future();  // 消费者获取结果的通道
    
    // 启动工作线程（转移 promise 所有权）
    std::thread t(asyncDivision, std::move(prom), 10, 3);
    t.detach();  // 分离线程（不等待结束）
    
    try {
        // 获取结果（阻塞直到完成）
        std::cout << "10/3 = " << fut.get() << std::endl;
    } catch(const std::exception& e) {
        // 捕获工作线程设置的异常
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    test_asyncDivision();
    return 0;
}
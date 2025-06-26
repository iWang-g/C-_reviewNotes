#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <numeric>
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

void use_jointhread() {
    // ================= 测试用例 1 =================
    // 验证：使用通用模板构造函数直接创建线程
    // 调用函数：template<typename Callable, typename ... Args> 
    //          explicit joining_thread(Callable&& func, Args&&... args)
    // 原理：
    //   1. 完美转发 lambda 表达式和参数 10
    //   2. 在成员初始化列表中构造 std::thread
    //   3. 线程立即启动执行
    joining_thread j1([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "in thread id " << std::this_thread::get_id()
                << " cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10);  // 参数 maxindex=10

    // ================= 测试用例 2 =================
    // 验证：使用显式构造函数接管已存在的线程
    // 调用函数：explicit joining_thread(std::thread t) noexcept
    // 原理：
    //   1. 创建临时 std::thread 对象（启动线程）
    //   2. 通过移动构造函数转移线程所有权
    //   3. 临时 thread 对象变为空状态
    joining_thread j2(std::thread([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "in thread id " << std::this_thread::get_id()
                << " cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10));  // 参数 maxindex=10

    // ================= 测试用例 3 =================
    // 验证：同上，但用于后续测试移动赋值
    // 调用函数：explicit joining_thread(std::thread t) noexcept
    // 设计目的：
    //   创建第三个线程，为测试移动赋值做准备
    joining_thread j3(std::thread([](int maxindex) {
        for (int i = 0; i < maxindex; i++) {
            std::cout << "in thread id " << std::this_thread::get_id()
                << " cur index is " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        }, 10));  // 参数 maxindex=10

    // ================= 测试用例 4 =================
    // 验证：移动赋值运算符的功能和线程安全
    // 调用函数：joining_thread& operator=(joining_thread&& other) noexcept
    // 执行步骤：
    //   1. 检查 j1 当前是否管理活跃线程（joinable()）
    //   2. 若存在活跃线程，调用 join() 等待其完成
    //   3. 将 j3 管理的线程所有权转移给 j1
    //   4. j3 变为空状态（不再管理任何线程）
    // 关键点：
    //   阻塞等待 j1 的原线程结束后才进行赋值
    //   确保线程资源安全转移
    j1 = std::move(j3);

    // ================= 析构阶段 =================
    // 函数结束时局部变量析构顺序（栈展开）：
    //   1. j3 析构：空状态 → 无操作
    //   2. j2 析构：调用 ~joining_thread() → join() 等待线程结束
    //   3. j1 析构：调用 ~joining_thread() → join() 等待线程结束
    // 设计验证：
    //   RAII 确保所有线程在退出作用域前完成
}

void cpu_concurrency_count()
{
    /*
        std::thread::hardware_concurrency()函数用于获取系统支持的并发线程数，
        通常在多核系统中返回CPU核心数。然而，返回值仅为提示，可能不准确，尤其是在无法获取信息时会返回0。
    */
    cout << thread::hardware_concurrency() << endl;
}


/*
* @brief 模拟实现并行计算
* @template param 
*   Iterator：任意迭代器类型（支持随机访问）
*   T：累加结果类型
* @param first/last：数据范围
*        init：累加初始值
*/
template<typename Iterator, typename T>
struct accumulate_block {
    void operator()(Iterator first, Iterator last, T& result) {
        result = std::accumulate(first, last, result); // 使用accumulate需包含#include <numeric>
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init)
{
    // 1. 输入验证
    unsigned long const length = std::distance(first, last); // distance 计算两个迭代器之间的元素数量
    if(!length)
        return init; // 处理空序列情况：直接返回初始值
    
    // 2. 线程数决策
    unsigned long const min_per_thread = 25; // 每个线程最少处理25个元素
    // 计算理论最大线程数（向上取整）
    unsigned long const max_threads = (length + min_per_thread - 1) / min_per_thread;
    //获取硬件支持的并发线程数（可能返回0）
    unsigned long const hardware_threads = thread::hardware_concurrency();

    // 确定最终线程数：
    //   - 优先使用硬件并发数（已测16）
    //   - 若硬件信息不可用则默认2线程
    //   - 不超过理论最大线程数
    unsigned long const num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);

    // 3. 任务划分
    unsigned long const block_size = length / num_threads; // 每块基础大小
    // 预分配结果存储（每个线程对应一个结果）
    vector<T> results(num_threads);
    // 创建工作线程容器（主线程会处理最后一块，所以少一个线程）
    vector<thread> threads(num_threads - 1);

    // 4. 并行任务分发
    Iterator block_start = first; // 起始迭代器
    for(unsigned long i = 0; i < (num_threads - 1); i++) {
        Iterator block_end = block_start;
        // 移动迭代器确定当前块结束位置（向容器末尾方向移动）
        std::advance(block_end, block_size);

        /* 
        * 此时数据块范围：
        *   - block_start: 当前块起始迭代器（包含）
        *   - block_end:   当前块结束迭代器（不包含）
        * 有效数据范围: [block_start, block_end)
        */

        // 启动线程处理当前数据块：
        //   - 使用accumulate_block函数对象
        //   - 传递数据范围（block_start到block_end）
        //   - std::ref确保结果引用传递
        threads[i] = thread(
            accumulate_block<Iterator, T>(), // 函数对象
            block_start, block_end, // 传递给函数的参数（数据范围）
            std::ref(results[i]) // 结果存储位置（引用传递）
        );

        // 更新下一块的起始位置
        block_start = block_end;
    }

    // 5. 主线程处理最后一块
    // 处理剩余元素（最后一块可能包含额外元素）
    accumulate_block<Iterator, T>()(
        block_start, last, // 最后一个数据块范围
        results[num_threads - 1] // 存储位置
    );

    // 6. 线程同步
    // 等待所有工作线程完成
    for(auto& entry : threads)
        entry.join();
    
    // 7. 结果合并
    // std::accumulate 是串行累加聚合工具，用于对迭代器区间 [first, last) 内的元素，以初始值 init 为起点，通过二元操作（默认是加法）聚合结果
    return std::accumulate(results.begin(), results.end(), init);
}

void use_parallel_accumulate()
{
    vector<int> vec;
    for(int i = 0; i < 10000; i++) {
        vec.push_back(i);
    }
    int sum = 0;
    sum = parallel_accumulate<vector<int>::iterator, int>(vec.begin(), vec.end(), sum);
    cout << "sum is " << sum << endl;
}

int main()
{
    use_parallel_accumulate();

    return 0;
}
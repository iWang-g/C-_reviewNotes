#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

void task1()
{
    cout << "无参线程正在执行任务..." << endl;

    // 让当前线程休眠2秒
    this_thread::sleep_for(chrono::seconds(2));

    cout << "无参线程休眠结束" << endl;
}

void task2(string str)
{
    cout << "含参线程正在执行任务..." << endl;
    cout << "str:" << str << endl;
    cout << "含参线程休眠结束" << endl;
}

void create_thread1()
{
    thread t(task1); // 创建线程
    t.join(); // 主线程等待子线程结束
}

void create_thread2(string str)
{
    thread t(task2, str);
    t.join();
}

class background_task
{
public:
    void operator()(string str) {
        cout << "str:" << str << endl;
    }
};

// 仿函数作为参数
void create_thread3()
{
    // 1.
    background_task bt;
    thread t(bt, "wg");
    t.join();
    
    // 2. 给 background_task() 加括号是为强制编译器将 background_task() 解析为创建临时对象的表达式，而非函数声明以消除语法歧义。
    std::thread t2((background_task()), "wg");
    t2.join();

    // 3. 可使用 {} 方式初始化
    std::thread t3{background_task(), "wg"};
    t3.join();
}

// lambda 表达式也可以作为线程的参数传递给 thread
void create_thread4()
{
    thread t([](string str){
        cout << "str:" << str << endl;
    }, "wg");

    t.join();
}

void change_param(int& param)
{
    param++;
}

void ref_oops(int some_param)
{
    std::cout << "before change , param is " << some_param << std::endl;
    
    // 当通过std::thread创建线程时，构造函数会将所有参数按值拷贝到线程的私有存储中（底层通过decay-copy实现）
    // 当线程要调用的回调函数参数为引用类型时，需要将参数显示转化为引用对象传递给线程的构造函数
    thread t(change_param, ref(some_param));
    t.join();
    
    std::cout << "after change , param is " << some_param << std::endl;
}

// 绑定类成员函数
class X
{
public:
    void do_lengthy_work() {
        std::cout << "do_lengthy_work " << std::endl;
    }
};

void bind_class_oops()
{
    X m_x;
    /*
        核心原理：非静态成员函数（如 X::do_lengthy_work）隐式依赖对象的 this 指针。
        创建线程时需同时提供 成员函数指针 和 对象实例指针，线程内部才能正确调用 m_x.do_lengthy_work()
    */
    thread t(&X::do_lengthy_work, &m_x);
    t.join();
}

// 使用 move 操作
void deal_unique(unique_ptr<int> p)
{
    std::cout << "unique ptr data is " << *p << std::endl;
    (*p)++;
    std::cout << "after unique ptr data is " << *p << std::endl;
}

void move_oops()
{
    auto p = make_unique<int>(100);
    thread t(deal_unique, move(p));
    t.join();

    //不能再使用p了，p已经被move废弃
    // std::cout << "after unique ptr data is " << *p << std::endl;
}

int main()
{
    move_oops();

    return 0;
}
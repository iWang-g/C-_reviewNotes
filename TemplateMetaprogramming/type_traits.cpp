#include <iostream>
#include <type_traits>
#include <vector>
#include <list>
#include <utility>

// 1. 编译时类型判断
template <typename T>
void printType() {
    // 编译时条件语句if constexpr，它允许在编译时根据条件选择不同的代码分支，从而简化模板元编程
    if constexpr (std::is_integral_v<T>) {
        std::cout << "Integral type\n";
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "Floating point type\n";
    } else {
        std::cout << "Other type\n";
    }
}

// 2. 编译期条件控制 
// (1) std::enable_if: 用于基于条件启用/禁用函数模板重载
// 仅当T为整型时启用此重载
template <typename T>
std::enable_if_t<std::is_integral_v<T>, void> process(T x) {
    std::cout << "Processing integer: " << x << "\n"; 
}

// 禁用整型的通用版本
template <typename T>
std::enable_if_t<!std::is_integral_v<T>, void> process(T x) {
    std::cout << "Unsupported type\n";
}

// (2) std::conditional: 在编译期选择类型
// 根据大小选择容器
template <typename T>
using Container = std::conditional<(sizeof(T) <= 4), std::vector<T>, std::list<T>>;

// 3. 编译期计算
// (1) std::integral_constant: 封装编译期常量值（如整数、布尔值）为类型
// 计算阶乘
template <size_t N>
struct Factorial:std::integral_constant<size_t, N * Factorial<N - 1>::value> {};

template <>
struct Factorial<0>:std::integral_constant<size_t, 1> {};

// (2) constexpr 函数
// 在编译期执行计算，如编译期斐波那契数列
constexpr size_t fibonacci(size_t n) {
    return (n <= 1) ? n : fibonacci(n - 1) + fibonacci(n - 2);
}

// 4. 序列生成与索引技巧
// std::integer_sequence: 生成编译期整数序列（C++14引入）
// 展开参数包
template <typename T, typename... Args>
void printArgs(T&& first, Args&&... args) {
    std::cout << first;
    ((std::cout << ", " << args), ...);
    std::cout << "\n";
}

template <typename... Args, size_t... ls>
void invokePrint(std::index_sequence<ls...>, Args&&... args) {
    printArgs((std::forward<Args>(args))...);
}

// g++ .\type_traits.cpp -std=c++17
int main() 
{
    printType<int>();      // 输出: Integral type
    printType<double>();   // 输出: Floating point type
    printType<std::string>(); // 输出: Other type

    process(10);
    process(3.14);

    Container<int> c1;  // 使用 vector (sizeof(int)=4)
    Container<double> c2; // 使用 list (sizeof(double)=8)
    std::cout << "Container for int: " << typeid(c1).name() << "\n"; // 获取类型信息
    std::cout << "Container for double: " << typeid(c2).name() << "\n";

    std::cout << Factorial<5>::value << "\n"; // 输出: 120 (5! = 120)

    constexpr size_t fib10 = fibonacci(10); // 编译期计算
    static_assert(fib10 == 55, "Fibonacci(10) should be 55");

    invokePrint(std::index_sequence<0, 1, 2>{}, "Hello", 42, 3.14);
    return 0;
}
#include <iostream>
#include <string>

// 1. 定义Traits模板（默认分类为Other）
template <typename T>
struct TypeTraits {
    static const char* category() { return "Other"; }
};

// 2. 特化数值类型（int, double等）
template <>
struct TypeTraits<int> {
    static const char* category() { return "Number"; }
};

template <>
struct TypeTraits<double> {
    static const char* category() { return "Number"; }
};

// 3. 特化字符串类型
template <>
struct TypeTraits<std::string> {
    static const char* category() { return "String"; }
};

// 4. 利用Traits分发处理逻辑
template <typename T>
void process(const T& value) {
    std::cout << "Processing " << TypeTraits<T>::category()
              << ": " << value << std::endl;
}

int main() {
    process(42);              // 输出: Processing Number: 42
    process(3.14);            // 输出: Processing Number: 3.14
    process(std::string("Hello")); // 输出: Processing String: Hello
    process("Raw char*");     // 输出: Processing Other: Raw char*
    return 0;
}
#include <iostream>
#include <tuple>
#include <sstream>
#include <string>

template<typename Tuple, size_t... I>
void stringToTupleHelper(const std::string& str, Tuple& tuple, std::index_sequence<I...>) {
    std::istringstream iss(str);
    (void)std::initializer_list<int>{ (iss >> std::get<I>(tuple), iss.ignore(), 0)... };
}

template<typename... Args>
std::tuple<Args...> stringToTuple(const std::string& str) {
    std::tuple<Args...> result;
    stringToTupleHelper(str, result, std::index_sequence_for<Args...>());
    return result;
}

int main() {
    std::string inputString = "42,3.14,Hello";
    auto myTuple = stringToTuple<int, double, std::string>(inputString);

    std::cout << std::get<0>(myTuple) << std::endl;  // 输出：42
    std::cout << std::get<1>(myTuple) << std::endl;  // 输出：3.14
    std::cout << std::get<2>(myTuple) << std::endl;  // 输出：Hello

    return 0;
}
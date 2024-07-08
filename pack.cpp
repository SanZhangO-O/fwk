// #include "out/base.pb.h"
// #include "rpc.hpp"

#include <iostream>
#include <type_traits>
#include <tuple>
using namespace std;

template <typename Tuple, std::size_t N>
struct TuplePrinter {
    static void print(const Tuple& t) {
        TuplePrinter<Tuple, N-1>::print(t);
        std::cout << std::get<N-1>(t) << std::endl;
    }
};

template <typename Tuple>
struct TuplePrinter<Tuple, 0> {
    static void print(const Tuple& t) {}
};

template <typename... Args>
void printTuple(const std::tuple<Args...>& t) {
    using TupleType = typename std::remove_reference<decltype(t)>::type;
    TuplePrinter<TupleType, std::tuple_size<TupleType>::value>::print(t);
}

int main() {
    std::tuple<int, double, std::string> myTuple(42, 3.14, "Hello");
    printTuple(myTuple);
    return 0;
}
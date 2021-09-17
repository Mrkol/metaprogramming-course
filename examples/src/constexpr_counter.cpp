#include <iostream>
#include <type_traits>

template<int N>
struct Tag {
    friend constexpr bool loophole(Tag<N>);
};

template<int N>
struct Writer {
    friend constexpr bool loophole(Tag<N>) {
        return true;
    }
    static constexpr int value = N;
};

template<int N, auto>
constexpr int reader(float) {
    return Writer<N>::value;
}

template<int N = 0, auto v, bool = loophole(Tag<N>{})>
constexpr int reader(int) {
    return reader<N + 1, v>(int{});
}

template<auto v = []() {}, int R = reader<0, v>(int{})>
constexpr int next() {
    return R;
}

constexpr int a = next();
constexpr int b = next();
constexpr int c = next();


int main() {
    std::cout << a << b << c;

    return 0;
}


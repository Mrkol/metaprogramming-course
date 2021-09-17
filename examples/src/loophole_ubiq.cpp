#include <iostream>
#include <type_traits>
#include <vector>
#include <tuple>

template <class T, class... Args>
concept AggregateConstructibleFrom = requires(Args... args) {
    T{ args... };
};

template <std::size_t I>
struct UbiqConstructor {
    template <class Type>
    constexpr operator Type&() const noexcept;
};

template <class T, std::size_t... I>
constexpr size_t countFieldsImpl(std::index_sequence<I...>) {
    return sizeof...(I) - 1;
}

template <class T, std::size_t... I> requires
    AggregateConstructibleFrom<T, UbiqConstructor<I>...>
constexpr size_t countFieldsImpl(std::index_sequence<I...>) {
    return countFieldsImpl<T>(std::index_sequence<0, I...>{});
}

template <class T>
constexpr size_t countFields() {
    return countFieldsImpl<T>(std::index_sequence<>{});
}


template <class T, int N>
struct Tag {
    friend auto loophole(Tag<T, N>);    
};

template<class T, int N, class F>
struct LoopholeSet {
    friend auto loophole(Tag<T, N>) { return F{}; };
};

template <class T, std::size_t I>
struct LoopholeUbiq {
    template <class Type>
    constexpr operator Type() const noexcept {
        LoopholeSet<T, I, Type> unused{};
        return {};
    };
};

template<class T, int N>
struct LoopholeGet {
    using Type = decltype(loophole(Tag<T, N>{}));
};


template <class T, size_t... Is>
constexpr auto asTupleImpl(std::index_sequence<Is...>) {
    constexpr T t{ LoopholeUbiq<T, Is>{}... };
    return std::tuple<typename LoopholeGet<T, Is>::Type...>{};
}

template <class T>
constexpr auto asTuple() {
    constexpr size_t num_fields = countFields<T>();
    return asTupleImpl<T>(std::make_index_sequence<num_fields>{});
}

// -----------------------

struct A {
    int a;
    char b;
};

struct B {
    A a;
    A b;
    float c;
};

template <class T>
void printType() {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

template <class T>
void printType(T) {
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

int main() {
    printType(asTuple<B>());

    return 0;
}

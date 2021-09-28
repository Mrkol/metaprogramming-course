#pragma once

#include <utility>


template<class... Ts>
struct TList {};


template<class TL>
concept TypeList = requires(TL t)
                   {
                       ([]<class... Ts> (TList<Ts...>) {})(t);
                   };

static_assert(!TypeList<int>);
static_assert(TypeList<TList<int, float, float>>);

template<class TL>
struct asdf;

template<class T, class... Ts>
struct asdf<TList<T, Ts...>> { using Tail = TList<Ts...>; };

template<TypeList TL, class T>
using Prepend = decltype( []<class... Ts> (TList<Ts...>) { return TList<T, Ts...>{}; } (TL{}) );


template<TypeList TL, class T>
using Append = decltype( []<class... Ts> (TList<Ts...>) { return TList<Ts..., T>{}; } (TL{}) );

template<class... Ts>
void print_tuple(std::tuple<Ts...> t)
{
    [&]<std::size_t... Is> (std::index_sequence<Is...>)
    {
        (
            [&]<std::size_t I>(std::integral_constant<std::size_t, I>)
            {
                // I
                std::cout << std::get<I>(t);
                //
                //
                //
            } (std::integral_constant<std::size_t, Is>{}), ...);

    } (std::make_index_sequence<sizeof...(Ts)>{});
}


//template<class T, std::size_t N>
//struct Helper
//{
//    using Type = T;
//    template<class U, std::size_t M>
//    auto operator,(Helper<U, M>)
//    {
//        if constexpr (N == 0)
//            return *this;
//        else
//            return Helper<U, N - 1>{};
//    }
//};
//
//template<TypeList TL, std::size_t N>
//using Get = typename decltype([]<class... Ts> (TList<Ts...>) {
//        return (..., Helper<Ts, N>{});
//        // (Helper<T1, N> + Helper<T2, N>) + Helper<T3, N>
//    } (TL{}) )::Type;

template <typename T, size_t I>
struct OverloadProvider {
    static T GetType(std::integral_constant<size_t, I>);
};

template<TypeList TL>
constexpr std::size_t Size = []<class... Ts> (TList<Ts...>) { return sizeof...(Ts); } (TL{});

template <TypeList TL, class K>
struct OverloadCollector;

template <typename... Ts, size_t... Is>
struct OverloadCollector<TList<Ts...>, std::index_sequence<Is...>>
        : OverloadProvider<Ts, Is>...
// int float double
// : OverloadProvider<int, 0>, OverloadProvider<float, 1>, OverloadProvider<double, 2>
{
    using OverloadProvider<Ts, Is>::GetType...;
};

template<TypeList TL, std::size_t N>
using Get = decltype(
        OverloadCollector<TL, std::make_index_sequence<Size<TL>>>
            ::GetType(std::integral_constant<std::size_t, N>{}));

template<class T>
concept DependentFalse = false;

//static_assert(DependentFalse<Get<TList<float, int, bool>, 1>>);
static_assert(std::is_same_v<Get<TList<float, int, bool>, 0>, float>);
static_assert(std::is_same_v<Get<TList<float, int, bool>, 1>, int>);
static_assert(std::is_same_v<Get<TList<float, int, bool>, 2>, bool>);

template<class L, class R> struct Pair {};

// BOOST.HANA

template<std::size_t N, class... Ts>
auto SuffixHelper(TList<Ts...> t)
{
    if constexpr (sizeof...(Ts) <= N)
    {
        return t;
    }
    else
    {
        return ([]<class U, class... Us>(TList<U, Us...>) { return SuffixHelper<N>(TList<Us...>{}); } )(t);
    }
}

template<TypeList TL, std::size_t N>
using Suffix = decltype(SuffixHelper<N>(TL{}));

static_assert(std::same_as<
        Suffix<TList<int, float, bool, char>, 2>,
        TList<bool, char>>);

template<class T>
struct TypeTag {};

template<class T>
class AbstractHandler
{
public:
    virtual void handle(TypeTag<T>, T) = 0;
};

template<class... Ts>
class AbstractEventSet : public AbstractHandler<Ts>...
{
public:
    using Events = TList<Ts...>;

    using AbstractHandler<Ts>::handle...;

    template<class T>
    void handle(T&& t)
    {
        handle(TypeTag<T>{}, std::forward<T>(t));
    }
};

struct KeyDownEventInfo { int which; };
struct MouseEventInfo { float x, y; };




template<class T, class Base>
class ConcreteHandler : public Base
{
public:
    virtual void handle(TypeTag<T>, T) override
    {

    }
};

template<class Base>
class ConcreteHandler<KeyDownEventInfo, Base> : public Base
{
public:
    virtual void handle(TypeTag<KeyDownEventInfo>, KeyDownEventInfo) override
    {

    }
};

// дано: template<class> class K,     K::handle


template<TypeList TL, template<class, class> class K, class TL2>
struct Intermediate;

template<class... Ts, template<class, class> class K>
struct Intermediate <TList<Ts...>, K, TList<>>
        : public AbstractEventSet<Ts...> {};

template<class TL, template<class, class> class K, class T, class... Ts>
struct Intermediate <TL, K, TList<T, Ts...>>
    : K<T, Intermediate<TL, K, TList<Ts...>>>
{

};

template<template<class, class> class K, class AbstractEventSet>
class ConcreteEventSet
        : public Intermediate<typename AbstractEventSet::Events, K, typename AbstractEventSet::Events>
{

};



void foo()
{
    using InputEvents = AbstractEventSet<KeyDownEventInfo, MouseEventInfo>;
    using InputEventHandler = ConcreteEventSet<ConcreteHandler, InputEvents>;
    InputEvents* evset = new InputEventHandler;

    evset->handle(KeyDownEventInfo{ 42 });
}

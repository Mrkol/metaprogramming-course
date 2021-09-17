#pragma once


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

template<class T, std::size_t N>
struct Helper
{
    using Type = T;
    template<class U, std::size_t M>
    auto operator+(Helper<U, M>)
    {
        if constexpr (N == 0)
            return *this;
        else
            return Helper<U, N - 1>{};
    }
};

template<TypeList TL, std::size_t N>
using Get = typename decltype([]<class... Ts> (TList<Ts...>) {
        return (... + Helper<Ts, N>{});
        // (Helper<T1, N> + Helper<T2, N>) + Helper<T3, N>
    } (TL{}) )::Type;


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

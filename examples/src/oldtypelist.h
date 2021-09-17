#pragma once
#include <type_traits>


struct Nil {
};

template<typename Head, typename Tail>
struct Cons {
    using head = Head;
    using tail = Tail;
};

template<class TL>
concept TypeList = std::same_as<TL, Nil> ||
                   requires(TL t)
                   {
                       ([]<class Head, class Tail> (Cons<Head, Tail>) {})(t);
                   };


template<typename T, typename Tlist>
using Prepend = Cons<T, Tlist>;

namespace details {
    template<typename T, typename Tlist>
    struct AppendImpl {
        using Type =
            Cons
            < typename Tlist::head
            , typename AppendImpl<T, typename Tlist::tail>::Type
            >;
    };

    template<typename T>
    struct AppendImpl<T, Nil> {
        using Type = Cons<T, Nil>;
    };
}


template<typename T, typename TList>
using Append = typename details::AppendImpl<T, TList>::Type;

static_assert(std::is_same_v
        < Append
          < int
          , Cons
            < float
            , Cons
              < int
              , Nil
              >
            >
          >
        , Cons
          < float
          , Cons
            < int
            , Cons
              < int
              , Nil
              >
            >
          >
        >);

template<class T>
constexpr bool DependentFalse = false;


namespace details {
    template<typename Tlist, typename Tlist_other>
    struct ConcatImpl {
        using Type =
            Cons
            < typename Tlist::head
            , typename ConcatImpl<typename Tlist::tail, Tlist_other>::Type
            >;
    };

    template<typename Tlist_other>
    struct ConcatImpl<Nil, Tlist_other> {
        using Type = Tlist_other;
    };
}

template<typename Tlist, typename Tlist_other>
using Concat = typename details::ConcatImpl<Tlist, Tlist_other>::Type;

static_assert(std::is_same_v
    < Concat<Cons<int, Cons<float, Nil>>, Cons<float, Cons<bool, Nil>>>
    , Cons<int, Cons<float, Cons<float, Cons<bool, Nil>>>>
    >);

namespace details {
    template<typename... Ts>
    struct BuildImpl {
        using type = Nil;
    };

    template<typename Head, typename... Tail>
    struct BuildImpl<Head, Tail...> {
        using type = Cons<Head, typename BuildImpl<Tail...>::type>;
    };
}

template<class... Ts>
using MakeTList = typename details::BuildImpl<Ts...>::type;

static_assert(std::is_same_v<MakeTList<float, int, char>,
                             Cons<float, Cons<int, Cons<char, Nil>>>>);


template<typename Tlist>
constexpr size_t Length = 1 + Length<typename Tlist::tail>;

template<>
constexpr size_t Length<Nil> = 0;


static_assert(Length<MakeTList<float, int, char>> == 3);
static_assert(Length<Nil> == 0);


namespace details {
    template<typename Head, typename Tail>
    struct SplitImpl {
        using gen = SplitImpl<typename Tail::head, typename Tail::tail>;
        using init = Cons<Head, typename gen::init>;
        using last = typename gen::last;
    };

    template<typename Head>
    struct SplitImpl<Head, Nil> {
        using init = Nil;
        using last = Head;
    };
}

template <typename List>
using Last = typename details::SplitImpl<typename List::head,
                                         typename List::tail>::last;
template <typename List>
using Init = typename details::SplitImpl<typename List::head,
                                         typename List::tail>::init;

static_assert(std::is_same_v<Last<MakeTList<int, float, bool>>, bool>);
static_assert(std::is_same_v<Init<MakeTList<int, float, bool>>,
        MakeTList<int, float>>);

namespace details {
    template <template <typename> typename F, typename List>
    struct MapImpl {
        using Type = Nil;
    };
    template <template <typename> typename F, typename Head, typename Tail>
    struct MapImpl<F, Cons<Head, Tail>> {
        using Type = Cons<F<Head>, typename MapImpl<F, Tail>::Type>;
    };
}

template <template <typename> typename F, typename List>
using Map = typename details::MapImpl<F, List>::Type;

template<class T>
using Starred = T*;

static_assert(std::is_same_v<Map<Starred, MakeTList<int, float, bool>>, MakeTList<int*, float*, bool*>>);

namespace details {
    template <typename T, bool Odd, typename Tlist>
    struct IntersperseImpl {};

    template <typename T, typename Tlist>
    struct IntersperseImpl<T, true, Tlist> {
        using Type = Cons
            < T
            , typename IntersperseImpl<T, false, Tlist>::Type
            >;
    };

    template <typename T, typename Tlist>
    struct IntersperseImpl<T, false, Tlist> {
        using Type = Cons
            < typename Tlist::head
            , typename IntersperseImpl<T, true, typename Tlist::tail>::Type
            >;
    };

    template <typename T>
    struct IntersperseImpl<T, true, Nil> {
        using Type = Nil;
    };

    template <typename T>
    struct IntersperseImpl<T, false, Nil> {
        using Type = Nil;
    };
}

template <typename T, typename Tlist>
using Intersperse = typename details::IntersperseImpl<T, false, Tlist>::Type;

static_assert(std::is_same_v
    < MakeTList<int, char, float>
    , Intersperse<char, MakeTList<int, float>>
      >);

static_assert(std::is_same_v
    < MakeTList<int, char, float, char, double>
    , Intersperse<char, MakeTList<int, float, double>>
    >);

static_assert(std::is_same_v
    < MakeTList<int, char, float, char, double, char, size_t>
    , Intersperse<char, MakeTList<int, float, double, size_t>>
    >);

// reverse
namespace details {
    template<typename TList, typename Reversed>
    struct ReverseImpl {
        using Type = typename ReverseImpl<typename TList::tail, Cons<typename TList::head, Reversed>>::Type;
    };

    template<typename Reversed>
    struct ReverseImpl<Nil, Reversed> {
        using Type = Reversed;
    };
}

template<typename TList>
using Reverse = typename details::ReverseImpl<TList, Nil>::Type;

static_assert(std::is_same_v<MakeTList<int, float, double>,
            Reverse<MakeTList<double, float, int>>
        >);
// end reverse

namespace details {
    template <size_t N, class L, class R, bool = (Length<R> > N)>
    struct PrefixImpl;

    template <size_t N, class L, class RHead, class RTail>
    struct PrefixImpl<N, L, Cons<RHead, RTail>, true> {
        using gen = PrefixImpl<N, Append<L, RHead>, RTail>;
        using Left = typename gen::Left;
        using Right = typename gen::Right;
    };

    template <size_t N, class L, class R>
    struct PrefixImpl<N, L, R, false> {
        using Left = L;
        using Right = R;
    };
}

template<class List, std::size_t N>
using Prefix = typename details::PrefixImpl<Length<List> - N, Nil, List>;


// using Zip

template <typename T, typename U> struct Pair {
    using firstType = T;
    using secondType = U;
};

namespace details {

    template <typename Tlist, typename TList_other>
    struct ZipImpl {
        using Type = Cons<
                Pair<typename Tlist::head, typename TList_other::head>,
                typename ZipImpl<typename Tlist::tail, typename TList_other::tail>::Type
                >;
    };


    template <typename TList>
    struct ZipImpl<Nil, TList> {
        using Type = Nil;
    };

    template <typename Tlist>
    struct ZipImpl<Tlist, Nil> {
        using Type = Nil;
    };

    template <>
    struct ZipImpl<Nil, Nil> {
        using Type = Nil;
    };

} // namespace details

template <typename List, typename List_other>
using Zip = typename details::ZipImpl<List, List_other>::Type;

static_assert(std::is_same_v<
        Zip<MakeTList<int, float, char>, MakeTList<int, float, bool, bool>>,
        MakeTList<Pair<int, int>, Pair<float, float>, Pair<char, bool>>>
    );

static_assert(std::is_same_v<
        Zip<MakeTList<int, float, char>, MakeTList<int, float, bool>>,
        MakeTList<Pair<int, int>, Pair<float, float>, Pair<char, bool>>>
);


//using t = Zip<int, float>;

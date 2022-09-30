#include <utility>
#include <tuple>
#include <type_traits>
#include <concepts>
#include <cstddef>

// type list
template<class... Ts>
struct TypeTuple;


struct Nil;
template<class U, class T>
struct Cons;


// Cons<float, Cons<int, Cons<bool, Cons<void, Nil>>>>
// Cons<float, Cons<int, Cons<bool, void>>>
// TypeList = Nil || Cons<class HEAD, TypeList TAIL>

// вроде как, алиасы нельзя частично специализировать
// template<class T> using Append<Nil, T> = Cons<Nil, T>;
// 

namespace detail {

template<class TL, class T>
struct Append;

template<class T>
struct Append<Nil, T> {
  using Type = Cons<T, Nil>;
};

template<class U, class Tail, class T>
struct Append<Cons<U, Tail>, T> {
  using Type = Cons<U, typename Append<Tail, T>::Type>;
};

template<class... Ts>
struct PackToList {
  using Type = Nil;
};

template<class T, class... Ts>
struct PackToList<T, Ts...> {
    using Type = Cons<T, typename PackToList<Ts...>::Type>;
};

}  // namespace detail


template<class... Ts>
using PackToList = typename detail::PackToList<Ts...>::Type;

// (TypeList, class) -> TypeList
template<class TL, class T>
using Append = typename detail::Append<TL, T>::Type;


static_assert(
  std::same_as
  < PackToList<float, double, int>
  , Cons
    < float
    , Cons
      < double
      , Cons<int, Nil>
      >
    >
  >
);

static_assert(
  std::same_as
  < Append
    < PackToList<float, double>
    , int
    >
  , PackToList<float, double, int>
  >);


// Concatenate +

// Concatenate +

// namespace detail {

template <typename TL1, typename TL2>
struct Concatenate;

template <typename T, typename Ts, typename TL2>
struct Concatenate<Cons<T, Ts>, TL2> {
  using Type = Cons<T, typename Concatenate<Ts, TL2>::Type>;
};

template <typename TL2>
struct Concatenate<Nil, TL2> {
  using Type = TL2;
};



// Nil || Cons<T, ...>
template <typename TL1, typename TL2>
using Concatenate = typename details::Concatenate<T1, TL2>::Type;
template <typename TL1, typename T>
using Append = typename details::Concatenate<T1, Cons<T, Nil>>::Type;

static_assert(
  std::same_as
  < Concatenate<Cons<float, Nil>, Cons<double, Nil>>
  , PackToList<float, double>
  >
);


// Head, Tail -- first element and all but first element  + 

namespace detail {
    template <class TL>
    struct Head;
    
    template <class T, class Ts>
    struct Head<Cons<T, Ts>> {
        using Type = T;
    };
    
    // Nil || Cons<T, Ts>
    template <class Tl>
    struct Tail;

    template <class T, class TL>
    struct Tail<Cons<T, TL>>{
        using Type = TL;
    };
    
} // namespace detail

template<class TL>
using Head = typename detail::Head<TL>::Type;

template<class TL>
using Tail = typename detail::Tail<TL>::Type;


static_assert(
    std::same_as
    < Head<Cons<void, Cons<int, Cons<bool, Nil>>>>
    , void
    >
);


static_assert(
    std::same_as
    < Tail<Cons<void, Cons<int, Cons<bool, Nil>>>>
    , Cons<int, Cons<bool, Nil>>
    >
);

// Length +

namespace detail {
  
  template<typename T>
  struct Length {
    static constexpr size_t value = 0;
  };
  
  template<typename T, typename TL>
  struct Length<Cons<T, TL>> {
    static constexpr size_t value = 1 + Length<TL>::value;
  };
  
  
}  // namespace detail

template<typename TL>
constexpr size_t Length = detail::Length<TL>::value;

static_assert(Length<PackToList<int, float, void>> == 3);

// Reverse +

namespace detail {

template<typename TLSource, typename TLResult = Nil>
struct Reverse;

template<typename Head, typename Tail, typename TLResult> 
struct Reverse<Cons<Head, Tail>, TLResult> {
    using Type =
      typename Reverse
      < Tail
      , Cons<Head, TLResult>
      >::Type;
};

template<typename TLResult> 
struct Reverse<Nil, TLResult> {
    using Type = TLResult;

};

}

// template<typename Head, typename Tail> 
// struct Reverse<Cons<Head, Tail>> {
//   using Type = typename Append<Reverse<Tail>, Head>::Type;
// }


template<typename TL>
using Reverse = typename detail::Reverse<TL>::Type;


template<class T>
constexpr bool DependentFalse = false;

// static_assert(DependentFalse<Reverse<PackToList<void, int>>>);

static_assert(std::same_as<Reverse<PackToList<void, int>>, PackToList<int, void>>);


// Map +

namespace detail {

template <template<class> class F, class TL> 
struct Map;

template <template<class> class F, class T, class TL>
struct Map<F, Cons<T, TL>>{
    using Type = Cons<F<T>, typename Map<F, TL>::Type>;
};

template<template<class>  class F>
struct Map<F, Nil> {
    using Type = Nil;
};
 
}

template <template<class> class F, class TL> 
using Map = typename detail::Map<F, TL>::Type;

template<class T>
using AddStar = T*;

static_assert(
  std::same_as
  < Map<AddStar, PackToList<int, void, bool>>
  , PackToList<int*, void*, bool*>
  >);


// Intersperse +

namespace detail {
  template<typename T, typename TL>
  struct Intersperse;
  
  template<typename T>
  struct Intersperse<T, Nil> {
    using Type = Nil;
  };

  template<typename T, typename U>
  struct Intersperse<T, Cons<U, Nil>> {
    using Type = Cons<U, Nil>;
  };

  template<typename T, typename Head, typename Tail>
  struct Intersperse<T, Cons<Head, Tail>> {
    using Type = Cons<Head, Cons<T, typename Intersperse<T, Tail>::Type>>;
  };
}

template<typename T, typename TL>
using Intersperse = typename detail::Intersperse<T, TL>::Type;

static_assert(
  std::same_as
  < Intersperse<char, PackToList<int, void, bool>>
  , PackToList<int, char, void, char, bool>
  >
);

// First+

// Первые N элементов TL. Если элементов меньше -- весь TL
namespace detail {
  template<size_t N, class TL>
  struct FirstN;
  
  // N, Cons
  template<size_t N, class T, class TL> requires (N > 0)
  struct FirstN<N, Cons<T, TL>> {
    using firstN = Cons<T, typename FirstN<N-1, TL>::firstN>;
  };

  template<size_t N, class TL> requires (N == 0) || std::same_as<TL, Nil>
  struct FirstN<N, TL> {
    using firstN = Nil;
  };
  
}


template<size_t N, class List>
using FirstN = typename detail::FirstN<N, List>::firstN;

static_assert(
  std::same_as
  < FirstN<3, PackToList<int, bool, char, void>>
  , PackToList<int, bool, char>
  >
);
static_assert(
  std::same_as
  < FirstN<3, PackToList<int, bool>>
  , PackToList<int, bool>
  >
);


// Last +

// Последние N элементов TL. Если их меньше -- весь TL

template<size_t N, class List>
using LastN = Reverse<FirstN<N, Reverse<List>>>;

static_assert(
  std::same_as
  < LastN<3, PackToList<int, bool, char, void>>
  , PackToList<bool, char, void>
  >
);






int main()
{
  
}

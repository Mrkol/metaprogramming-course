#pragma once

#include <type_traits>
#include <concepts>

#include "typelist.hpp"


// List that contains all of Ts
template<class... Ts>
using PackToList = void; // TODO

// Append T to the end of TL
template<class TL, class T>
using Append = void; // TODO


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


// Concatenate

template <typename TL1, typename TL2>
using Concatenate = void; // TODO

static_assert(
  std::same_as
  < Concatenate<Cons<float, Nil>, Cons<double, Nil>>
  , PackToList<float, double>
  >
);


// Head, Tail

// First element of TL
template<class TL>
using Head = void; // TODO

// All elements of TL but the first one
template<class TL>
using Tail = void; // TODO


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

template<typename TL>
constexpr size_t Length = 0; // TODO

static_assert(Length<PackToList<int, float, void>> == 3);

// Reverse

template<typename TL>
using Reverse = void; // TODO


static_assert(std::same_as<Reverse<PackToList<void, int>>, PackToList<int, void>>);


// Map

template <template<class> class F, class TL>
using Map = void; // TODO

template<class T>
using AddStar = T*;

static_assert(
  std::same_as
  < Map<AddStar, PackToList<int, void, bool>>
  , PackToList<int*, void*, bool*>
  >);


// Intersperse

template<typename T, typename TL>
using Intersperse = void; // TODO

static_assert(
  std::same_as
  < Intersperse<char, PackToList<int, void, bool>>
  , PackToList<int, char, void, char, bool>
  >
);

// First

// Первые N элементов TL. Если элементов меньше -- весь TL
template<size_t N, class List>
using FirstN = void; // TODO

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


// Last

// Последние N элементов TL. Если их меньше -- весь TL
template<size_t N, class List>
using LastN = void; // TODO

static_assert(
  std::same_as
  < LastN<3, PackToList<int, bool, char, void>>
  , PackToList<bool, char, void>
  >
);


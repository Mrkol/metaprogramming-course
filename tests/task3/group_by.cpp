#include <type_tuples.hpp>
#include <type_lists.hpp>

#include <concepts>
#include <cstdint>
#include <type_traits>


using type_tuples::TTuple;

using type_lists::ToTuple;
using type_lists::FromTuple;
using type_lists::Map;
using type_lists::Take;
using type_lists::Cycle;
using type_lists::Iterate;
using type_lists::GroupBy;


template<class L, class R>
struct CompareBySize { static constexpr bool Value = sizeof(L) == sizeof(R); };

static_assert(
  std::same_as
  < ToTuple
    < Map
      < ToTuple
      , GroupBy
        < CompareBySize
        , FromTuple
          < TTuple<int32_t, uint32_t, int16_t, uint16_t, uint8_t, bool, char, int16_t>
          >
        >
      >
    >
  , TTuple
    < TTuple<int32_t, uint32_t>
    , TTuple<int16_t, uint16_t>
    , TTuple<uint8_t, bool, char>
    , TTuple<int16_t>
    >
  >);

static_assert(
  std::same_as
  < ToTuple<Map<ToTuple, Take<5, GroupBy<CompareBySize, Cycle<FromTuple<TTuple<char, bool, int, int, char>>>>>>>
  , TTuple
    < TTuple<char, bool>
    , TTuple<int, int>
    , TTuple<char, char, bool>
    , TTuple<int, int>
    , TTuple<char, char, bool>
    >
  >);

template<class T>
using Starred = T*;

template<class, class>
struct FalseOP {
  constexpr static bool Value = false;
};

template<class L, class R>
struct IsSame {
  constexpr static bool Value = std::is_same_v<L, R>;
};

static_assert(
  std::same_as
  < ToTuple
    < Map
      < ToTuple
      , Take
        < 4
        , GroupBy<FalseOP, Iterate<Starred, int>>
        >
      >
    >
  , TTuple
    < TTuple<int>
    , TTuple<int*>
    , TTuple<int**>
    , TTuple<int***>
    >
  >
);


int main() {
  return 0;
}

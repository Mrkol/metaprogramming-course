#include <type_lists.hpp>
#include <testing/metafunctions.hpp>
#include <testing/assert.hpp>

#include <gtest/gtest.h>

#include <cstdint>


// If you can find a better way of doing this, contact me
// (pretty sure it's impossible due to no higher kind polymorphism :/ )
CURRY(type_lists, Take, std::size_t, type_lists::TypeList);
CURRY(type_lists, Map, template<class> class, type_lists::TypeList);



template<class T>
  using Starred = T*;

template<class L, class R>
using MaxBySize = std::conditional_t<(sizeof(L) >= sizeof(R)), L, R>;

template<class T>
struct Fits { static constexpr bool Value = sizeof(T) <= 4; };



void checkSimpleOps() {
  using type_tuples::TTuple;
  using type_lists::Cons;
  using type_lists::Nil;

  using type_lists::Repeat;
  static_assert(
    requires {
      requires type_lists::TypeSequence<Repeat<int>>;
      requires type_lists::TypeSequence<typename Repeat<int>::Tail>;
      requires type_lists::TypeSequence<typename Repeat<int>::Tail::Tail>;
      requires std::same_as<typename Repeat<int>::Head, int>;
      requires std::same_as<typename Repeat<int>::Tail::Head, int>;
      requires std::same_as<typename Repeat<int>::Tail::Tail::Head, int>;
    });

  using type_lists::ToTuple;
  static_assert(
    std::same_as
    < ToTuple< Cons<int, Cons<bool, Cons<float, Nil>>> >
    , TTuple<int, bool, float>
    >);

  using type_lists::FromTuple;
  static_assert(
    std::same_as
    < ToTuple<FromTuple<TTuple<int, bool, float, char>>>
    , TTuple<int, bool, float, char>
    >);

  using type_lists::Take;
  static_assert(
    std::same_as
    < ToTuple<Take<3, Repeat<int>>>
    , TTuple<int, int, int>
    >);

  static_assert(
    std::same_as
    < ToTuple<Take<2, FromTuple<TTuple<int, bool, float, double>>>>
    , TTuple<int, bool>
    >);

  static_assert(
    std::same_as
    < ToTuple<Take<0, FromTuple<TTuple<int, bool, float, double>>>>
    , TTuple<>
    >);

  static_assert(
    std::same_as
    < ToTuple<Take<10, FromTuple<TTuple<int, bool, float, double>>>>
    , TTuple<int, bool, float, double>
    >);

  using type_lists::Drop;
  static_assert(
    std::same_as
    < ToTuple<Drop<3, FromTuple<TTuple<float, int, float, int>>>>
    , TTuple<int>
    >);

  static_assert(
    std::same_as
    < ToTuple<Drop<0, FromTuple<TTuple<float, int, float, int>>>>
    , TTuple<float, int, float, int>
    >);

  static_assert(
    std::same_as
    < ToTuple<Drop<10, FromTuple<TTuple<float, int, float, int>>>>
    , TTuple<>
    >);

  using type_lists::Replicate;
  static_assert(
    std::same_as
    < ToTuple<Replicate<3, int>>
    , TTuple<int, int, int>
    >);

  using type_lists::Iterate;
  static_assert(std::same_as
    < ToTuple<Take<4, Iterate<Starred, int>>>
    , TTuple<int, int*, int**, int***>
    >);

  using type_lists::Cycle;
  static_assert(std::same_as
    < ToTuple<Take<5, Cycle<FromTuple<TTuple<int, bool>>>>>
    , TTuple<int, bool, int, bool, int>
    >);

}

void checkTransforms() {
  using type_tuples::TTuple;
  using type_lists::ToTuple;
  using type_lists::FromTuple;
  using type_lists::Repeat;
  using type_lists::Take;
  using type_lists::Nil;
  using type_lists::Cycle;


  using type_lists::Map;
  static_assert(
    std::same_as
    < ToTuple<Map<Starred, FromTuple<TTuple<int, bool, float>>>>
    , TTuple<int*, bool*, float*>
    >);

  static_assert(
    std::same_as
    < ToTuple<Map<Starred, Nil>>
    , TTuple<>
    >);

  static_assert(
    std::same_as
    < ToTuple
      < Map
      < ToTuple
      , Take
        < 3
        , Map
        < CTake<3>::template Curry
        , Repeat<Repeat<float>>
        >
        >
      >
      >
    , TTuple
      < TTuple<float, float, float>
      , TTuple<float, float, float>
      , TTuple<float, float, float>
      >
    >);


  using type_lists::Filter;
  static_assert(std::same_as
    < ToTuple<Filter<Fits, FromTuple<TTuple<int, bool, std::uint64_t, char, std::int64_t, short>>>>
    , TTuple<int, bool, char, short>
    >);

  static_assert(std::same_as
    < ToTuple<Filter<Fits, Nil>>
    , TTuple<>
    >);
  
  static_assert(std::same_as
    < ToTuple<Take<5, Filter<Fits, Cycle<FromTuple<TTuple<int, bool, std::uint64_t>>>>>>
    , TTuple<int, bool, int, bool, int>
    >);
}

void checkReducers() {
  using type_lists::ToTuple;
  using type_lists::Map;
  using type_tuples::TTuple;
  using type_lists::FromTuple;
  using type_lists::Cycle;
  using type_lists::Take;
  using type_lists::Nil;

  using type_lists::Scanl;
  static_assert(
    std::same_as
    < ToTuple<Scanl
          < MaxBySize
          , char
          , FromTuple<TTuple<char, bool, short, char, int, short, std::int64_t, int>>
          >>
    , TTuple<char, char, char, short, short, int, int, std::int64_t, std::int64_t>
    >);

  using type_lists::Foldl;
  static_assert(
    std::same_as
    < Foldl
      < MaxBySize
      , char
      , FromTuple<TTuple<char, bool, short, char, int, short, std::int64_t, int>>
      >
    , std::int64_t
    >);

  static_assert(
    std::same_as
    < Foldl
      < MaxBySize
      , char
      , Nil
      >
    , char
    >);
}

void checkSublists() {
  using type_tuples::TTuple;
  using type_lists::ToTuple;
  using type_lists::FromTuple;
  using type_lists::Map;
  using type_lists::Nil;



  using type_lists::Inits;
  static_assert(
    std::same_as
    < ToTuple<Map<ToTuple, Inits<FromTuple<TTuple<int, char, bool, float>>>>>
    , TTuple
      < TTuple<>
      , TTuple<int>
      , TTuple<int, char>
      , TTuple<int, char, bool>
      , TTuple<int, char, bool, float>
      >
    >);

  static_assert(
    std::same_as
    < ToTuple<Map<ToTuple, Inits<Nil>>>
    , TTuple
      < TTuple<>
      >
    >);



  using type_lists::Tails;
  static_assert(
    std::same_as
    < ToTuple<Map<ToTuple, Tails<FromTuple<TTuple<int, char, bool, float>>>>>
    , TTuple
      < TTuple<int, char, bool, float>
      , TTuple<char, bool, float>
      , TTuple<bool, float>
      , TTuple<float>
      , TTuple<>
      >
    >);

  static_assert(
    std::same_as
    < ToTuple<Map<ToTuple, Tails<Nil>>>
    , TTuple
      < TTuple<>
      >
    >);
}



void checkZips() {
  using type_tuples::TTuple;
  using type_lists::Repeat;
  using type_lists::Take;
  using type_lists::ToTuple;
  using type_lists::FromTuple;
  using type_lists::Nil;



  using type_lists::Zip2;
  static_assert(
    std::same_as
    < ToTuple<Take<10, Zip2<Repeat<int>, Repeat<float>>>>
    , ToTuple<Take<10, Repeat<TTuple<int, float>>>>
    >);

  static_assert(
    std::same_as
    < ToTuple<Take<10, Zip2<FromTuple<TTuple<bool, char, int>>, Repeat<float>>>>
    , TTuple
      < TTuple<bool, float>
      , TTuple<char, float>
      , TTuple<int, float>
      >
    >);

  static_assert(
    std::same_as
    < ToTuple<Take<10, Zip2<FromTuple<TTuple<bool, char, int>>, Nil>>>
    , TTuple<>
    >);



  using type_lists::Zip;
  static_assert(
    std::same_as
    < ToTuple<Take<10, Zip<Repeat<int>, Repeat<float>, Repeat<double>>>>
    , ToTuple<Take<10, Repeat<TTuple<int, float, double>>>>
    >);
}

template<class T>
using TupTake3 = type_lists::ToTuple<type_lists::Take<3, T>>;

void checkCornercases() {
  using type_tuples::TTuple;

  using type_lists::Empty;
  using type_lists::Cycle;
  using type_lists::Nil;
  using type_lists::ToTuple;
  using type_lists::Map;
  using type_lists::Inits;
  using type_lists::Tails;
  using type_lists::Iterate;
  using type_lists::Take;

  static_assert(Empty<Cycle<Nil>>);

  static_assert(
    std::same_as
    < ToTuple
      < Take
        < 4
        , Iterate
          < Starred
          , int
          >
        >
      >
    , TTuple<int, int*, int**, int***>
    >
  );

  using SomeCyclesOfInts = Map<TupTake3, Map<Cycle, Inits<Iterate<Starred, int>>>>;
  static_assert(
    std::same_as
    < TupTake3<SomeCyclesOfInts>
    , TTuple
      < TTuple<>
      , TTuple<int, int, int>
      , TTuple<int, int*, int>
      >
    >
  );

  static_assert(
    std::same_as
    < ToTuple
      < Take
        < 4
        , Map
          < ToTuple
          , Inits
            < Iterate
              < Starred
              , int
              >
            >
          >
        >
      >
    , TTuple
      < TTuple<>
      , TTuple<int>
      , TTuple<int, int*>
      , TTuple<int, int*, int**>
      >
    >
  );

  static_assert(
    std::same_as
    < ToTuple
      < Take
        < 3
        , Map
          < TupTake3
          , Tails
            < Iterate
              < Starred
              , int
              >
            >
          >
        >
      >
    , TTuple
      < TTuple<int, int*, int**>
      , TTuple<int*, int**, int***>
      , TTuple<int**, int***, int****>
      >
    >
  );
}

TEST(TypeListsTest, JustWorks)
{
  checkSimpleOps();
  checkTransforms();
  checkReducers();
  checkSublists();
  checkZips();
  checkCornercases();
}

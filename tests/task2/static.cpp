#include <type_lists.hpp>
#include <commons/metafunctions.hpp>
#include <commons/assert.hpp>


namespace Tests {


// If you can find a better way of doing this, contact me
// (pretty sure it's impossible due to no higher kind polymorphism :/ )
CURRY(TypeLists, Take, std::size_t, TypeLists::TypeList);
CURRY(TypeLists, Map, template<class> class, TypeLists::TypeList);



template<class T>
    using Starred = T*;

template<class L, class R>
struct CompareBySize { static constexpr bool Value = sizeof(L) == sizeof(R); };

template<class L, class R>
using MaxBySize = std::conditional_t<(sizeof(L) >= sizeof(R)), L, R>;

template<class T>
struct Fits { static constexpr bool Value = sizeof(T) <= 4; };



void checkSimpleOps() {
    using TypeTuples::TTuple;
    using TypeLists::Cons;
    using TypeLists::Nil;

    using TypeLists::Repeat;
    static_assert(
        requires {
            requires TypeLists::TypeSequence<Repeat<int>>;
            requires TypeLists::TypeSequence<typename Repeat<int>::Tail>;
            requires TypeLists::TypeSequence<typename Repeat<int>::Tail::Tail>;
            requires std::same_as<typename Repeat<int>::Head, int>;
            requires std::same_as<typename Repeat<int>::Tail::Head, int>;
            requires std::same_as<typename Repeat<int>::Tail::Tail::Head, int>;
        });

    using TypeLists::ToTuple;
    static_assert(
        std::same_as
        < ToTuple< Cons<int, Cons<bool, Cons<float, Nil>>> >
        , TTuple<int, bool, float>
        >);

    using TypeLists::FromTuple;
    static_assert(
        std::same_as
        < ToTuple<FromTuple<TTuple<int, bool, float, char>>>
        , TTuple<int, bool, float, char>
        >);

    using TypeLists::Take;
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

    using TypeLists::Drop;
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

    using TypeLists::Replicate;
    static_assert(
        std::same_as
        < ToTuple<Replicate<3, int>>
        , TTuple<int, int, int>
        >);

    using TypeLists::Iterate;
    static_assert(std::same_as
        < ToTuple<Take<4, Iterate<Starred, int>>>
        , TTuple<int, int*, int**, int***>
        >);

    using TypeLists::Cycle;
    static_assert(std::same_as
        < ToTuple<Take<5, Cycle<FromTuple<TTuple<int, bool>>>>>
        , TTuple<int, bool, int, bool, int>
        >);

}

void checkTransforms() {
    using TypeTuples::TTuple;
    using TypeLists::ToTuple;
    using TypeLists::FromTuple;
    using TypeLists::Repeat;
    using TypeLists::Take;
    using TypeLists::Nil;


    using TypeLists::Map;
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


    using TypeLists::Filter;
    static_assert(std::same_as
        < ToTuple<Filter<Fits, FromTuple<TTuple<int, bool, uint64_t, char, int64_t, short>>>>
        , TTuple<int, bool, char, short>
        >);

    static_assert(std::same_as
        < ToTuple<Filter<Fits, Nil>>
        , TTuple<>
        >);

}

void checkReducers() {
    using TypeLists::ToTuple;
    using TypeLists::Map;
    using TypeTuples::TTuple;
    using TypeLists::FromTuple;
    using TypeLists::Cycle;
    using TypeLists::Take;
    using TypeLists::Nil;


    using TypeLists::GroupBy;
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



    using TypeLists::Scanl;
    static_assert(
        std::same_as
        < ToTuple<Scanl
                  < MaxBySize
                  , char
                  , FromTuple<TTuple<char, bool, short, char, int, short, int64_t, int>>
                  >>
        , TTuple<char, char, char, short, short, int, int, int64_t, int64_t>
        >);

    using TypeLists::Foldl;
    static_assert(
        std::same_as
        < Foldl
          < MaxBySize
          , char
          , FromTuple<TTuple<char, bool, short, char, int, short, int64_t, int>>
          >
        , int64_t
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
    using TypeTuples::TTuple;
    using TypeLists::ToTuple;
    using TypeLists::FromTuple;
    using TypeLists::Map;
    using TypeLists::Nil;



    using TypeLists::Inits;
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



    using TypeLists::Tails;
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
    using TypeTuples::TTuple;
    using TypeLists::Repeat;
    using TypeLists::Take;
    using TypeLists::ToTuple;
    using TypeLists::FromTuple;
    using TypeLists::Nil;



    using TypeLists::Zip2;
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



    using TypeLists::Zip;
    static_assert(
        std::same_as
        < ToTuple<Take<10, Zip<Repeat<int>, Repeat<float>, Repeat<double>>>>
        , ToTuple<Take<10, Repeat<TTuple<int, float, double>>>>
        >);
}


void checkIntegerSequences() {
    using TypeLists::ToTuple;
    using TypeLists::Take;

    // NATS

    static_assert(
      std::same_as
      < ToTuple<Take<10, Nats>>
      , VTuple<int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9>
      >);

    // FIB

    static_assert(
        std::same_as
        < ToTuple<Take<32, Fib>>
        , VTuple
          < int
          , 0
          , 1
          , 1
          , 2
          , 3
          , 5
          , 8
          , 13
          , 21
          , 34
          , 55
          , 89
          , 144
          , 233
          , 377
          , 610
          , 987
          , 1597
          , 2584
          , 4181
          , 6765
          , 10946
          , 17711
          , 28657
          , 46368
          , 75025
          , 121393
          , 196418
          , 317811
          , 514229
          , 832040
          , 1346269
          >
        >);

    // PRIMES

    static_assert(
      std::same_as
      < ToTuple<Take<16, Primes>>
      , VTuple
        < int
        , 2
        , 3
        , 5
        , 7
        , 11
        , 13
        , 17
        , 19
        , 23
        , 29
        , 31
        , 37
        , 41
        , 43
        , 47
        , 53
        >
      >);
    }


}

int main() {
    Tests::checkSimpleOps();
    Tests::checkTransforms();
    Tests::checkReducers();
    Tests::checkSublists();
    Tests::checkZips();
    Tests::checkIntegerSequences();
    return 0;
}

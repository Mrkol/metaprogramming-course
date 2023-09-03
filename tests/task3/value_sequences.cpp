#include <concepts>
#include <type_lists.hpp>
#include <value_types.hpp>
#include <fun_value_sequences.hpp>


using value_types::VTuple;
using type_lists::ToTuple;
using type_lists::Take;

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


int main() { 
  return 0;
}

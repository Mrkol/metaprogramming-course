#include <iostream>
#include <memory>
#include <concepts>
#include <utility>
#include <type_traits>
#include <array>
#include <variant>
#include <any>
#include <tuple>

template <class T>
constexpr bool DepFalse = false;

#define FOR_TEMPLATE(Is)                        \
  [&]<size_t... Is>(std::index_sequence<Is...>) \
  {                                             \
    ([&]() {

#define END_FOR_TEMPLATE(SIZE) \
  }() ,  ...);               \
  }                            \
  (std::make_index_sequence<SIZE>{});

template <int N>
struct Tag
{
  friend auto loophole(Tag<N>);
};
template <int N>
struct Loophole
{
  friend auto loophole(Tag<N>) {};
};

template <size_t I = 0, auto L = []() {}>
consteval size_t counter()
{
  if constexpr (requires { loophole(Tag<I>{}); }) {
    return counter<I + 1, L>();
  } else {
    (void) Loophole<I>{};
    return I;
  }
}

std::array<int, counter()> a;
std::array<int, counter()> b;
std::array<int, counter()> c;

template<auto V>
auto helper() { return __PRETTY_FUNCTION__; }

template<class V>
auto helper() { return __PRETTY_FUNCTION__; }

struct Foo
{
  int a;
  int b;
  int c;
};

int main()
{
  std::cout << a.size() << '\n';
  std::cout << b.size() << '\n';
  std::cout << c.size() << '\n';
  std::tuple<int, float, double, char, std::string> tuple{0, 42.f, 420.0, 'A', "Hi"};

  FOR_TEMPLATE(I)
  {
    std::cout << std::get<I>(tuple) << '\n';
  }
  END_FOR_TEMPLATE(std::tuple_size_v<decltype(tuple)>);

  std::cout << helper<&Foo::a>() << '\n';

  return 0;
}

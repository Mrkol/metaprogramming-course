#include <iostream>
#include <memory>
#include <concepts>
#include <utility>
#include <type_traits>
#include <array>
#include <variant>
#include <any>
#include <tuple>
#include <memory>
#include <ranges>


template<class T>
struct Tag {};

template<class Interface>
struct CreatorInterface {
  virtual std::unique_ptr<Interface> create(Tag<Interface>) = 0;
};

template<class... Interfaces>
struct AbstractFactory : private CreatorInterface<Interfaces>... {
  using CreatorInterface<Interfaces>::create...;

  virtual ~AbstractFactory() = default;
};

struct Cat { virtual ~Cat() = default; };
struct Dog { virtual ~Dog() = default; };
struct Cow { virtual ~Cow() = default; };

struct Sphinx : Cat {};
struct Spaniel : Dog {};
struct BlackAngus : Cow {};

template<class... Ts>
struct TypeTuple {};

template<class Implementations, class Interfaces, class FinalParent>
struct ConcreteCreator : FinalParent {};

template
  < class Implementation, class... Implementations
  , class Interface, class... Interfaces
  , class FinalParent
  >
struct ConcreteCreator
  < TypeTuple<Implementation, Implementations...>
  , TypeTuple<Interface, Interfaces...>
  , FinalParent
  >
  : ConcreteCreator
    < TypeTuple<Implementations...>
    , TypeTuple<Interfaces...>
    , FinalParent
    > {
  std::unique_ptr<Interface> create(Tag<Interface>) override {
    return std::make_unique<Implementation>();
  }
};

template<class Interfaces, class Implementations>
struct ConcreteFactory;

template<class... Interfaces, class... Implementations>
struct ConcreteFactory
  < TypeTuple<Interfaces...>
  , TypeTuple<Implementations...>
  >
  : ConcreteCreator
    < TypeTuple<Implementations...>
    , TypeTuple<Interfaces...>
    , AbstractFactory<Interfaces...>
    > {
};


template<size_t I>
struct ValueTag {};

template<class T, size_t I>
struct TupleElement {
  T t;

  T& get(ValueTag<I>) {
    return t;
  }

  T& get(Tag<T>) {
    return t;
  }
};

template<class, class... Ts>
struct Tuple1;

template<class... Ts, size_t... Is>
struct Tuple1<std::index_sequence<Is...>, Ts...>
  : private TupleElement<Ts, Is>... {
  using TupleElement<Ts, Is>::get...;
};

template<class... Ts>
using Tuple = Tuple1<std::make_index_sequence<sizeof...(Ts)>, Ts...>;



template<class... Ts>
struct Tuple2 {};

template<class T, class... Ts>
struct Tuple2<T, Ts...> : private Tuple2<Ts...> {
  template<size_t I>
  auto& get() {
    if constexpr (I == 0) return t;
    else return Tuple<Ts...>::template get<I - 1>();
  }

  T t;
};

template<size_t I, class Tuple>
decltype(auto) get(Tuple&& t) {
  return std::forward<Tuple>(t).get(ValueTag<I>{});
}

template<class T, class Tuple>
decltype(auto) get(Tuple&& t) {
  return std::forward<Tuple>(t).get(Tag<T>{});
}


int main() {
  std::unique_ptr<AbstractFactory<Cat, Dog, Cow>> factory =
    std::make_unique<
      ConcreteFactory
      < TypeTuple<Cat, Dog, Cow>
      , TypeTuple<Sphinx, Spaniel, BlackAngus>
      >
    >();

  auto cat = factory->create(Tag<Cat>{});
  std::cout << typeid(*cat).name() << std::endl;

  Tuple<int, int, float> tuple{};

  get<float>(tuple) = 4;

  std::cout << get<2>(tuple) << std::endl;

  return 0;
}

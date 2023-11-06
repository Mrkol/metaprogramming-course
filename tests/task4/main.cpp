#include <FixedString.hpp>
#include <PolymorphicMapper.hpp>

#include <commons/assert.hpp>

#include <optional>
#include <string_view>
#include <type_traits>
#include <memory>


using std::operator""sv;

static_assert(std::is_trivially_copyable_v<FixedString<256>>);
static_assert(std::is_same_v<FixedString<256>, decltype("smth"_cstr)>);
static_assert("some text"_cstr == "some text"sv);
static_assert(FixedString<128>{"some text", 4} == "some"sv);

class Animal {
public:
  virtual ~Animal() = default;
};

class Cat : public Animal {};
class Cow : public Animal {};
class Dog : public Animal {};
class StBernard : public Dog {};
class Horse : public Animal {};
class RaceHorse : public Horse {};

void checkWithInts() {
  using MyMapper =
    PolymorphicMapper
    < Animal, int
    , Mapping<Cat, 2>
    , Mapping<Dog, 3>
    >;

  std::unique_ptr<Animal> dog{new Dog()};
  std::unique_ptr<Animal> cow{new Cow()};
  std::unique_ptr<Animal> cat{new Cat()};

  MPC_REQUIRE(nullopt, MyMapper::map(*cow));
  MPC_REQUIRE(eq, MyMapper::map(*cat).value(), 2);
  MPC_REQUIRE(eq, MyMapper::map(*dog).value(), 3);
}

void checkWithStrings() {
  using MyMapper =
    PolymorphicMapper
    < Animal, FixedString<256>
    , Mapping<StBernard, "Baaark"_cstr>
    , Mapping<Cat, "Meow"_cstr>
    , Mapping<Dog, "Bark"_cstr>
    , Mapping<Horse, "Neigh"_cstr>
    >;

  std::unique_ptr<Animal> dog{new Dog()};
  std::unique_ptr<Animal> st_bernard{new StBernard()};
  std::unique_ptr<Animal> cow{new Cow()};
  std::unique_ptr<Animal> cat{new Cat()};
  std::unique_ptr<Animal> race_horse{new RaceHorse()};

  MPC_REQUIRE(nullopt, MyMapper::map(*cow));
  MPC_REQUIRE(eq, MyMapper::map(*cat).value(), "Meow"sv);
  MPC_REQUIRE(eq, MyMapper::map(*dog).value(), "Bark"sv);
  MPC_REQUIRE(eq, MyMapper::map(*st_bernard).value(), "Baaark"sv);
  MPC_REQUIRE(eq, MyMapper::map(*race_horse).value(), "Neigh"sv);
}

void checkEmpty() {
  using MyMapper = PolymorphicMapper<Animal, int>;

  std::unique_ptr<Animal> dog{new Dog()};
  std::unique_ptr<Animal> cow{new Cow()};
  std::unique_ptr<Animal> cat{new Cat()};

  MPC_REQUIRE(nullopt, MyMapper::map(*cow));
  MPC_REQUIRE(nullopt, MyMapper::map(*cat));
  MPC_REQUIRE(nullopt, MyMapper::map(*dog));
}

void checkStray() {
  using MyMapper = PolymorphicMapper<Animal, int, Mapping<Dog, 42>, Mapping<Animal, 1>>;
  std::unique_ptr<Animal> dog{new Dog()};
  std::unique_ptr<Animal> cat{new Cat()};

  MPC_REQUIRE(eq, MyMapper::map(*dog).value(), 42);
  MPC_REQUIRE(eq, MyMapper::map(*cat).value(), 1);
}

int main() {
  checkWithInts();
  checkWithStrings();
  checkEmpty();
  checkStray();
}

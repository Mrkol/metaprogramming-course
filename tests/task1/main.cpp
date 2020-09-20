#include <string_mapper.hpp>
#include <commons/assert.hpp>

#include <optional>
#include <string_view>
#include <type_traits>
#include <memory>
#include <iostream>


using std::operator""sv;

static_assert(std::is_literal_type_v<String<256>>);
static_assert(std::is_same_v<String<256>, decltype("smth"_cstr)>);
static_assert("some text"_cstr == "some text"sv);
static_assert(String<128>{"some text", 4} == "some"sv);

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
    using MyMapper = ClassMapper<
        Animal, int,
        Mapping<Cat, 2>,
        Mapping<Dog, 3>
    >;

    std::shared_ptr<Animal> dog{new Dog()};
    std::shared_ptr<Animal> cow{new Cow()};
    std::shared_ptr<Animal> cat{new Cat()};

    ensure(nullopt, MyMapper::map(*cow));
    ensure(eq, *MyMapper::map(*cat), 2);
    ensure(eq, *MyMapper::map(*dog), 3);
}

void checkWithStrings() {
    using MyMapper = ClassMapper<
        Animal, String<256>,
        Mapping<StBernard, "Baaark"_cstr>,
        Mapping<Cat, "Meow"_cstr>,
        Mapping<Dog, "Bark"_cstr>,
        Mapping<Horse, "Neigh"_cstr>
    >;

    std::shared_ptr<Animal> dog{new Dog()};
    std::shared_ptr<Animal> st_bernard{new StBernard()};
    std::shared_ptr<Animal> cow{new Cow()};
    std::shared_ptr<Animal> cat{new Cat()};
    std::shared_ptr<Animal> race_horse{new RaceHorse()};

    ensure(nullopt, MyMapper::map(*cow));
    ensure(eq, *MyMapper::map(*cat), "Meow"sv);
    ensure(eq, *MyMapper::map(*dog), "Bark"sv);
    ensure(eq, *MyMapper::map(*st_bernard), "Baaark"sv);
    ensure(eq, *MyMapper::map(*race_horse), "Neigh"sv);
}

int main() {
    checkWithInts();
    checkWithStrings();
}

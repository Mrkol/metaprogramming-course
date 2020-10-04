#include <string_mapper.hpp>
#include <commons/concepts.hpp>

#include <concepts>
#include <optional>
#include <iostream>


template <class M, class Base, class To>
concept Mapper = mpg::Complete<M> && std::is_same_v<std::optional<To>, decltype(M::map(std::declval<const Base&>()))>;


class Animal {
public:
    virtual ~Animal() = default;
};

class Cat : public Animal {};
class Dog : public Animal {};

class Val {};
struct ExtVal : public Val {};

using MyMapper1 = ClassMapper<
    Animal, Val,
    Mapping<Cat, Val{}>,
    Mapping<Dog, ExtVal{}>
>;

using MyMapper2 = ClassMapper<
    Animal, int,
    Mapping<Cat, 2>,
    Mapping<Dog, Val{}>
>;


int main() {
    Cat cat;
    MyMapper1::map(cat);
//    MyMapper2::map(cat);
}

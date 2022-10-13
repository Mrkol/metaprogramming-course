#include <PolymorphicMapper.hpp>

class Animal {
public:
    virtual ~Animal() = default;
};

class Cat : public Animal {};
class Dog : public Animal {};

struct Val {
    /* implicit */ operator int() const noexcept {
        return 0;
    }
};

using MyMapper = PolymorphicMapper<
    Animal, int,
    Mapping<Cat, 2>,
    Mapping<Dog, Val{}>
>;


int main() {
    Cat cat;
    MyMapper::map(cat);
}

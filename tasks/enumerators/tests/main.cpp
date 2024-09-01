#include <EnumeratorTraits.hpp>

#include <testing/assert.hpp>

#include <gtest/gtest.h>

#include <iostream>
#include <string_view>
#include <utility>


enum class Shape { // : int
    SQUARE, CIRCLE = 5, LINE, POINT = -2
};

// check compile time capabilities
static_assert(EnumeratorTraits<Shape>::size() == 4);
static_assert(EnumeratorTraits<Shape>::nameAt(0) == "POINT");
static_assert(EnumeratorTraits<Shape>::at(0) == Shape::POINT);
static_assert(EnumeratorTraits<Shape>::at(1) == Shape::SQUARE);
static_assert(EnumeratorTraits<Shape>::at(3) == Shape::LINE);



#define ue(enumerator) std::pair(enumerator, std::string_view(#enumerator))
#define se(enumeration, enumerator) std::pair(enumeration::enumerator, std::string_view(#enumerator))

template <class Enum, size_t MAXN=512, std::same_as<Enum>... Args> requires std::is_enum_v<Enum>
void check(std::string_view name, std::pair<Args, std::string_view>... enumerators) {
    using Traits = EnumeratorTraits<Enum, MAXN>;
    auto expected = std::array<std::pair<Enum, std::string_view>, sizeof...(Args)>{enumerators...};

    std::cout << "Test for " << name << ": "; std::cout.flush();
    MPC_REQUIRE(eq, expected.size(), Traits::size());
    for (size_t i = 0; i < expected.size(); ++i) {
        std::cout << i << " "; std::cout.flush();
        MPC_REQUIRE(eq, expected[i].first, Traits::at(i));
        MPC_REQUIRE(eq, expected[i].second, Traits::nameAt(i));
    }
    std::cout << "OK" << std::endl;
}


// check extreme values (unsigned)
enum Fruit : unsigned char {
    APPLE, BANANA = 12, MELON = 255
};

// check extreme values (signed)
enum Vegetable : signed char {
    TOMATO, CUCUMBER = -128, ONION = 127
};

// check extreme underlying types (unsigned)
enum class Quests : unsigned long long {
    DozoR = 18, Encounter, CX
};

// check extreme underlying types (signed)
enum class Countries : long long {
    SriLanka
};

// check empty enums
enum class ScopedEmpty {}; // : int
enum UnscopedEmpty : long long {};


TEST(EnumeratorTest, JustWorks)
{
    check<Shape>("Shape", se(Shape, POINT), se(Shape, SQUARE), se(Shape, CIRCLE), se(Shape, LINE));
    check<Fruit>("Fruit", ue(APPLE), ue(BANANA), ue(MELON));
    check<Vegetable>("Vegetable", ue(CUCUMBER), ue(TOMATO), ue(ONION));

    check<Quests, 20>("Quests", se(Quests, DozoR), se(Quests, Encounter), se(Quests, CX));
    check<Countries, 20>("Countries", se(Countries, SriLanka));

    check<ScopedEmpty>("ScopedEmpty");
    check<UnscopedEmpty>("UnscopedEmpty");
}

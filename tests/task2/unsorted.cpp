#include <PolymorphicMapper.hpp>

#include <commons/assert.hpp>

#include <optional>
#include <type_traits>
#include <memory>


struct Animal {
  virtual ~Animal() = default;
};

struct Cat : Animal {};
struct Okayu : Cat {};

struct Dog : Animal {};
struct CavalierKingCharlesSpaniel : Dog {};
struct Korone : CavalierKingCharlesSpaniel {};

struct StBernard : Dog {};
struct Chihuahua : Dog {};
struct SleepingChihuahua : Chihuahua {};
struct AngryChihuahua : Chihuahua {};

struct Cow : Animal {};
struct Fauna : Cow {};

struct Horse : Animal {};
struct RaceHorse : Horse {};


template<class... Ts, size_t... Is>
void testImpl(std::index_sequence<Is...>)
{
  using MyMapper =
    PolymorphicMapper
    < Animal, size_t
    , Mapping<Ts, Is>...
    >;

  ([]()
  {
    std::unique_ptr<Animal> animal{new Ts()};
    MPC_REQUIRE(eq, MyMapper::map(*animal), Is);
  }(), ...);
}


template<class... Ts, size_t... Is>
void test() {
  testImpl<Ts...>(std::make_index_sequence<sizeof...(Ts)>{});
}

int main() {
  // Simple tests
  test
    < Korone
    , CavalierKingCharlesSpaniel
    , Dog
    >();

  test
    < Dog
    , Korone
    >();

  test
    < Dog
    , CavalierKingCharlesSpaniel
    , Korone
    >();    
  
  // Evil tests
  test
    < Cat
    , Okayu
    , Dog
    , CavalierKingCharlesSpaniel
    , Korone
    , StBernard
    , Chihuahua
    , SleepingChihuahua
    , AngryChihuahua
    , Cow
    , Fauna
    , Horse
    , RaceHorse
    >();

  test
    < Dog
    , RaceHorse
    , Okayu
    , Cat
    , CavalierKingCharlesSpaniel
    , AngryChihuahua
    , Chihuahua
    , StBernard
    , SleepingChihuahua
    , Horse
    , Korone
    , Cow
    , Fauna
    >();
    
  test
    < StBernard
    , RaceHorse
    , Chihuahua
    , Fauna
    , Okayu
    , SleepingChihuahua
    , CavalierKingCharlesSpaniel
    , Cow
    , Korone
    , Horse
    , AngryChihuahua
    , Cat
    , Dog
    >();
  
  test
    < AngryChihuahua
    , Korone
    , Chihuahua
    , Okayu
    , CavalierKingCharlesSpaniel
    , Dog
    , Cat
    , SleepingChihuahua
    , StBernard
    , Fauna
    , RaceHorse
    , Horse
    , Cow
    >();
  
  test
    < RaceHorse
    , SleepingChihuahua
    , Dog
    , Korone
    , AngryChihuahua
    , Cat
    , Horse
    , Fauna
    , Okayu
    , Chihuahua
    , CavalierKingCharlesSpaniel
    , StBernard
    , Cow
    >();
  
  test
    < Cow
    , CavalierKingCharlesSpaniel
    , AngryChihuahua
    , Okayu
    , Korone
    , RaceHorse
    , Chihuahua
    , Horse
    , Cat
    , Fauna
    , StBernard
    , SleepingChihuahua
    , Dog
    >();

  test
    < Chihuahua
    , Fauna
    , Korone
    , Cow
    , Horse
    , RaceHorse
    , Okayu
    , Dog
    , Cat
    , StBernard
    , AngryChihuahua
    , CavalierKingCharlesSpaniel
    , SleepingChihuahua
    >();
  
  test
    < Cow
    , Chihuahua
    , Korone
    , Fauna
    , Dog
    , RaceHorse
    , SleepingChihuahua
    , StBernard
    , AngryChihuahua
    , Cat
    , CavalierKingCharlesSpaniel
    , Okayu
    , Horse
    >();
}

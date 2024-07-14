#include <commons/assert.hpp>
#include <commons/RegularityWitness.hpp>

#include "mocks.hpp"

#include <Spy.hpp>


template <size_t pad_size = 0>
void testCopyable() {
  using mpc::detail::LoggerChecker;
  using mpc::detail::Counter;
  using mpc::detail::ValueLog;

  constexpr auto semiregular_opt = mpc::RegularityPolicy{};

  auto s = Spy<Counter<semiregular_opt>>{};
  MPC_REQUIRE(eq, s->isPositive(), false); // works without logger

  s = Spy{ Counter<semiregular_opt>(1) };
  MPC_REQUIRE(eq, s->x, 1); // construction and copy construction work

  LoggerChecker<semiregular_opt, pad_size> checker;
  s.setLogger(checker.getLogger());
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{});

  s->x = 0;
  s->isPositive() && s->x--;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{1, 1});

  (void) (s->x++ + s->x++);
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{2});

  auto s2 = s; // copy construction
  // logger must be copied too

  s2->isPositive() && s2->x--;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{2});

  s->x++;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{1});

  // what about move assignment?
  s = Spy { Counter<semiregular_opt>{} }; // no logger
  s->x++;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{});

  // which results are expected in the following expression?
  //  s->isPositive() && ((s = s2), 1);
  // ¯\_(ツ)_/¯

  // but this one is ok
  (s2->isPositive(), (s = s2)->isPositive());
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{ 1, 1 });

  LoggerChecker<semiregular_opt, pad_size> another_checker;
  s.setLogger(another_checker.getLogger()); // what about changing loggers?

  s->x++;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{});
  MPC_REQUIRE(eq, another_checker.pollValues(), ValueLog{ 1 });

  s2 = std::move(s); // what about move assignment?
  s2->x++;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{});
  MPC_REQUIRE(eq, another_checker.pollValues(), ValueLog{ 1 });

  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wself-assign-overloaded"
  (s2 = s2)->x++;
  #pragma clang diagnostic pop
  MPC_REQUIRE(eq, another_checker.pollValues(), ValueLog{ 1 });

  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wself-move"
  (s2 = std::move(s2))->x++;
  #pragma clang diagnostic pop
  MPC_REQUIRE(eq, another_checker.pollValues(), ValueLog{ 1 });

  auto s3 = Spy { Counter<semiregular_opt>{} }; // no logger
  s2 = s3; // copy assignment
  s2->x++;
  MPC_REQUIRE(eq, another_checker.pollValues(), ValueLog{});
}

template <size_t pad_size = 0>
void testMovable() {
  using mpc::detail::LoggerChecker;
  using mpc::detail::Counter;
  using mpc::detail::ValueLog;

  constexpr auto move_only_opt = mpc::RegularityPolicy{ .copy_constructor = false, .copy_assignment = false };

  auto s = Spy<Counter<move_only_opt>>{};
  MPC_REQUIRE(eq, s->isPositive(), false); // works without logger

  s = Spy{ Counter<move_only_opt>(1) };
  MPC_REQUIRE(eq, s->x, 1); // construction and move construction work

  LoggerChecker<move_only_opt, pad_size> checker;
  s.setLogger(checker.getLogger());
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{});

  s->x = 0;
  s->isPositive() && s->x--;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{1, 1});

  (void) (s->x++ + s->x++);
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{2});

  auto s2 = std::move(s); // move construction
  // logger must be moved too

  s2->isPositive() && s2->x--;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{2});

  // what about move assignment?
  s2 = Spy { Counter<move_only_opt>{} }; // no logger
  s2->x++;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{});

  s2.setLogger(checker.getLogger());
  
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wself-move"
  (s2 = std::move(s2))->x++;
  #pragma clang diagnostic pop
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{ 1 });
}

template <size_t pad_size = 0>
void testLvalueLogger() {
  using mpc::detail::LoggerChecker;
  using mpc::detail::Counter;
  using mpc::detail::ValueLog;

  constexpr auto semiregular_opt = mpc::RegularityPolicy{};

  auto s = Spy<Counter<semiregular_opt>>{};

  LoggerChecker<semiregular_opt, pad_size> checker;
  auto logger = checker.getLogger();
  s.setLogger(logger);
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{});

  s->x = 0;
  s->isPositive() && s->x--;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{1, 1});
}


void logFunction(unsigned int) {}

void testFunctionPointer() {
  using mpc::detail::LoggerChecker;
  using mpc::detail::Counter;
  constexpr auto semiregular_opt = mpc::RegularityPolicy{};

  auto s = Spy<Counter<semiregular_opt>>{};
  s.setLogger(&logFunction);
  s->x++;
}

int main() {
  testCopyable(); // will likely trigger SBO
  testCopyable<256>(); // will likely not trigger SBO

  testMovable(); // will likely trigger SBO
  testMovable<256>(); // will likely not trigger SBO

  testLvalueLogger(); // will likely trigger SBO
  testLvalueLogger<256>(); // will likely not trigger SBO

  testFunctionPointer();

  return 0;
}

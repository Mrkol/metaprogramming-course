#include <Spy.hpp>
#include <commons/RegularityWitness.hpp>
#include <concepts>


namespace mpc::detail {

template <class Logger, class T>
concept ProperSpyLogger = requires(Spy<T>& spy, Logger&& logger) {
  spy.setLogger(std::forward<Logger>(logger));
};

template <RegularityPolicy options>
struct EmptyLogger : RegularityWitness<options> {
  void operator ()(unsigned int) {}
};

}

void checkConceptPreservation() {
  using mpc::RegularityPolicy;
  using mpc::RegularityWitness;

  using Semiregular = RegularityWitness<RegularityPolicy{}>;
  using Copyable = RegularityWitness<RegularityPolicy{ .default_constructor = false }>;
  using NoCopy = RegularityWitness<RegularityPolicy{ .copy_constructor = false, .copy_assignment = false }>;
  using NoMove = RegularityWitness<RegularityPolicy{ .move_constructor = false, .move_assignment = false }>;
  using NoCopyMoveConstructor = RegularityWitness<RegularityPolicy{ .copy_constructor = false, .move_constructor = false }>;
  using NoCopyMoveAssignment = RegularityWitness<RegularityPolicy{ .copy_assignment = false, .move_assignment = false }>;
  using BadDestructor = RegularityWitness<RegularityPolicy{ .nothrow_destructor = false }>;

  static_assert(std::regular<Spy<int>>);

  static_assert(std::semiregular<Spy<Semiregular>>);
  static_assert(!std::regular<Spy<Semiregular>>);

  static_assert(std::copyable<Spy<Copyable>>);
  static_assert(!std::semiregular<Spy<Copyable>>);

  static_assert(std::movable<Spy<NoCopy>>);
  static_assert(!std::copyable<Spy<NoCopy>>);

  static_assert(std::semiregular<Spy<NoMove>>);
  static_assert(!std::regular<Spy<NoMove>>);

  static_assert(!std::movable<Spy<NoCopyMoveConstructor>>);
  static_assert(!std::movable<Spy<NoCopyMoveAssignment>>);
  static_assert(!std::movable<Spy<BadDestructor>>);
}

void checkLoggerSafety() {
  using mpc::RegularityPolicy;
  using mpc::RegularityWitness;
  using mpc::detail::ProperSpyLogger;
  using mpc::detail::EmptyLogger;

  constexpr auto copyable_opt = RegularityPolicy{ .default_constructor = false };
  constexpr auto move_only_opt = RegularityPolicy{ .copy_constructor = false, .copy_assignment = false };
  constexpr auto bad_destructor_opt = RegularityPolicy{ .nothrow_destructor = false };

  static_assert(ProperSpyLogger<EmptyLogger<copyable_opt>, RegularityWitness<copyable_opt>>);
  static_assert(!ProperSpyLogger<EmptyLogger<move_only_opt>, RegularityWitness<copyable_opt>>);
  static_assert(!ProperSpyLogger<EmptyLogger<bad_destructor_opt>, RegularityWitness<copyable_opt>>);

  static_assert(ProperSpyLogger<EmptyLogger<move_only_opt>, RegularityWitness<move_only_opt>>);
  static_assert(ProperSpyLogger<EmptyLogger<copyable_opt>, RegularityWitness<move_only_opt>>);
  static_assert(!ProperSpyLogger<EmptyLogger<bad_destructor_opt>, RegularityWitness<move_only_opt>>);
}

int main() {
  checkConceptPreservation();
  checkLoggerSafety();
  return 0;
}

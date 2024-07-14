#pragma once

namespace mpc {

struct RegularityPolicy {
  bool default_constructor = true;
  bool copy_constructor = true;
  bool move_constructor = true;
  bool copy_assignment = true;
  bool move_assignment = true;
  bool nothrow_destructor = true;
};

template <RegularityPolicy options>
struct RegularityWitness {
  RegularityWitness() requires (options.default_constructor) = default;
  RegularityWitness(const RegularityWitness&) requires (options.copy_constructor) = default;
  RegularityWitness(RegularityWitness&&) noexcept requires (options.move_constructor) = default;
  RegularityWitness& operator =(const RegularityWitness&) requires (options.copy_assignment) = default;
  RegularityWitness& operator =(RegularityWitness&&) noexcept requires (options.move_assignment) = default;
  ~RegularityWitness() noexcept(options.nothrow_destructor) {}
};

}

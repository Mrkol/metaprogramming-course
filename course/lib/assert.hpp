#pragma once

namespace mpc {

namespace detail {

struct AssertException
{ };

} // namespace detail

#define MPC_VERIFY(condition) \
do { \
  [[unlikely]] if (!(condition)) { \
    throw ::mpc::detail::AssertException{}; \
  } \
} while (false)

// Message is unused since its main purpose is stress relief.
#define MPC_VERIFYF(condition, message) \
do { \
  [[unlikely]] if (!(condition)) { \
    throw ::mpc::detail::AssertException{}; \
  } \
} while (false)

} // namespace mpc

#pragma once

namespace mpc {

namespace detail {

struct AssertException
{ };

} // namespace detail

// Message is unused since its main purpose is stress relief.
#define MPC_VERIFY(condition, message) \
    [[unlikely]] if (!(condition)) { \
        throw ::mpc::detail::AssertException{}; \
    } \
    static_assert(true);

} // namespace mpc
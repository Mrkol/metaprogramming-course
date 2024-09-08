#pragma once

#include <lib/assert.hpp>

#include "debug_trap.hpp"

#include <gtest/gtest.h>

#include <cstdlib>
#include <iostream>
#include <concepts>
#include <optional>
#include <string>

namespace mpc::detail {

void fail(const char* file, size_t line) {
  std::cerr << "Assertion failed: file \"" << file << "\", line " << line << std::endl;
#ifndef NDEBUG
  psnip_trap();
#else
  std::exit(1);
#endif
}

void assert_true(const char* file, size_t line, const std::convertible_to<bool> auto& v1) {
  if (!v1) {
    fail(file, line);
  }
}

void assert_eq(const char* file, size_t line, const auto& v1, const auto& v2) {
  assert_true(file, line, v1 == v2);
}

void assert_neq(const char* file, size_t line, const auto& v1, const auto& v2) {
  assert_true(file, line, v1 != v2);
}

template <class T>
void assert_nullopt(const char* file, size_t line, const std::optional<T>& v) {
  assert_true(file, line, !v.has_value());
}

}

#define MPC_REQUIRE(type, ...) mpc::detail::assert_##type(__FILE__, __LINE__, __VA_ARGS__)

#define MPC_CONCAT3_IMPL(x, y) x ## y
#define MPC_CONCAT3(x, y) MPC_CONCAT3_IMPL(x, y)
#define MPC_UNIQUE_NAME(x) MPC_CONCAT3(x, __LINE__)

// Most compilers are unable to properly explain why a
// static_assert(requires { ... }); failed, so we have
// to do a hack.
#define EXPECT_STATIC_TRUE(REQS)                       \
template<class T = void> requires (REQS)                    \
static constexpr bool MPC_UNIQUE_NAME(Checker) = true; \
static_assert(MPC_UNIQUE_NAME(Checker)<>);

#define EXPECT_RUNTIME_OK(expr) \
EXPECT_NO_THROW((expr));

#define EXPECT_RUNTIME_FAIL(expr) \
EXPECT_THROW((expr), ::mpc::detail::AssertException) \
  << "Consider using MPC_VERIFY for a given sanity check test (See \"lib/assert.hpp\")\n";
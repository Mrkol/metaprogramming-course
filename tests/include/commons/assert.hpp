#pragma once

#include <cstdlib>
#include <iostream>
#include <concepts>
#include <optional>


#define ensure(type, ...) mpg::detail::assert_##type(__FILE__, __LINE__, __VA_ARGS__)

namespace mpg::detail {

void fail(const char* file, size_t line) {
    std::cerr << "Assertion failed: file \"" << file << "\", line " << line << std::endl;
    std::abort();
}

void assert_true(const char* file, size_t line, const std::convertible_to<bool> auto& v1) {
    if (!v1) {
        fail(file, line);
    }
}

void assert_eq(const char* file, size_t line, const auto& v1, const auto& v2) {
    assert_true(file, line, v1 == v2);
}

template <class T>
void assert_nullopt(const char* file, size_t line, const std::optional<T>& v) {
    assert_true(file, line, !v.has_value());
}

}

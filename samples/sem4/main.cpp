#include <any>
#include <iostream>
#include <memory>
#include <concepts>
#include <utility>
#include <cassert>
#include "function.hpp"

void foo(Function<void(int)> f) {
  f(0);
}

struct FunctionalObject {
  void operator()(int) {
    std::cout << "Hello from struct!";
  };
};

void banana(int) {}

void test_function() {
  Function f(
    [called_once = false](int) mutable {
      if (!std::exchange(called_once, true))
        std::cout << "Hello!";
    });

  Function<void(int)> g(FunctionalObject{});
  g(0);

  foo(f);
  foo(f);
}

int main() {
  test_function();
  return 0;
}

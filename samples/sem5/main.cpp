#include <iostream>
#include <memory>
#include <concepts>
#include <utility>
#include <array>
#include <variant>
#include "any.hpp"



int main() {
  Any a = 42;

  std::cout << any_cast<int>(a) << std::endl;

  Any b = std::string("Hello!");
  a = b;

  std::cout << any_cast<std::string>(a) << std::endl;

  return 0;
}

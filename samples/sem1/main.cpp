#include <iostream>
#include <span>
#include <numeric>
#include <vector>
#include "binarySearch.hpp"

// size_t binarySearch(std::span<int const> data, int value) {
//     size_t left = 0;
//     size_t right = data.size();
//     while (right - left > 1) {
//         size_t middle = std::midpoint(left, right);
//         if (data[middle] <= value) {
//             left = middle;
//         }
//         else {
//             right = middle;
//         }
//     }
//     return left;
// }



// // + Ноль оверхеда
// // - Для каждого типа -- свой бинарный код
// // - Нужно явно создавать
// // - Код не даже парсится компилятором

// #define MAKE_BINARY_SEARCH(TYPE) \
// inline size_t binarySearchWithMacros(std::span<TYPE const> data, TYPE value) { \
//     size_t left = 0; \
//     size_t right = data.size(); \
//     while (right - left > 1) { \
//         size_t middle = std::midpoint(left, right); \
//         if (data[middle] <= value) { \
//             left = middle; \
//         } \
//         else { \
//             right = middle; \
//         } \
//     } \
//     return left; \
// }

// MAKE_BINARY_SEARCH(float);

// void bar()
// {
//   std::vector<float> vec;
//   (void) binarySearchWithMacros(vec, 4.2f);
// }

// // - Оверхед -- индерекация на вызов компаратора
// // - Байтики
// size_t binarySearch(std::span<std::byte const> data,
//   size_t oneObjectSize, const std::byte* value,
//   bool (*comparator)(const std::byte*, const std::byte*)) {

//   size_t left = 0;
//   size_t right = data.size() / oneObjectSize;
//   while (right - left > 1) {
//     size_t middle = std::midpoint(left, right);
//     if (comparator(&data[middle * oneObjectSize], value)) {
//       left = middle;
//     } else {
//       right = middle;
//     }
//   }
//   return left;
// }

// struct IComparable
// {
//   virtual bool compareTo(const IComparable&) const = 0;

//   virtual ~IComparable() = default;
// };

// struct Int : IComparable
// {
//   int value;

//   bool compareTo(const IComparable& other) const override
//   {
//     return value < dynamic_cast<const Int&>(other).value;
//   }
// };

// // - Оверхед -- две индерекации на вызов компаратора каждый раз
// // +- неоднородный контейнер
// // - массив указателей
// size_t binarySearch(std::span<IComparable const * const> data,
//   const IComparable& value) {
//   size_t left = 0;
//   size_t right = data.size();
//   while (right - left > 1) {
//     size_t middle = std::midpoint(left, right);
//     if (data[middle]->compareTo(value)) {
//       left = middle;
//     }
//     else {
//       right = middle;
//     }
//   }
//   return left;
// }


void foo()
{
  std::vector<float> vec;
  binarySearchTemplate<float>(vec, 4.2f);
}

template<class T>
struct GlobalStorage
{
  static std::vector<T> data = {};
};


template<template<class T> class storageTemplate>
struct Foo
{
  void foo()
  {
    storageTemplate<int>::data.push_back(42);
  }
};

Foo<GlobalStorage> bar;


int main(int argc, char** argv)
{ 
  bar.foo();
  return 0;
}

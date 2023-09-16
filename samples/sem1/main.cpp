#include <iostream>
#include <span>
#include <numeric>
#include <vector>

// Базовый алгоритм

size_t binarySearch(std::span<int const> data, int value) {
  size_t left = 0;
  size_t right = data.size();
  while (right - left > 1) {
    size_t middle = std::midpoint(left, right);
    if (data[middle] <= value) {
      left = middle;
    }
    else {
      right = middle;
    }
  }
  return left;
}

// Macro

// #define MAKE_BINARY_SEARCH(TYPE) \
// inline size_t binarySearchWithMacros(std::span<TYPE const> data, TYPE value) { \
//   size_t left = 0; \
//   size_t right = data.size(); \
//   while (right - left > 1) { \
//     size_t middle = std::midpoint(left, right); \
//     if (data[middle] <= value) { \
//       left = middle; \
//     } \
//     else { \
//       right = middle; \
//     } \
//   } \
//   return left; \
// }

// MAKE_BINARY_SEARCH(float);


// C-style

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


// ООП

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


// template<class T>
// T binarySearchTemplate(std::span<T const> data, T value) {
//   size_t left = 0;
//   size_t right = data.size();
//   while (right - left > 1) {
//     size_t middle = std::midpoint(left, right);
//     if (data[middle] <= value) {
//       left = middle;
//     }
//     else {
//       right = middle;
//     }
//   }
//   return left;
// }

int main(int argc, char** argv)
{
  return 0;
}

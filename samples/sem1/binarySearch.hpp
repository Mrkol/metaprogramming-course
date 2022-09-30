#include <span>
#include <numeric>





template<class T>
struct Searcher
{
    T binarySearchTemplate(std::span<T const> data, T value) {
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
};
// template<class T>
// T binarySearchTemplate(std::span<T const> data, T value);


template struct Searcher<float>;
//
extern template float Searcher<float>::binarySearchTemplate(
    std::span<float const> data, float value);



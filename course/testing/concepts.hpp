#pragma once

#include <type_traits>

namespace mpc {
namespace detail {

template <class T>
decltype(sizeof(T)) testIncomplete(T);

void testIncomplete(...);

}


template <class T>
concept Complete = !std::is_same_v<void, decltype(detail::testIncomplete(std::declval<T>()))>;

}

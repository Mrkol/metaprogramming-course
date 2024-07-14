#pragma once

#define CURRY(NameSpace, Name, K1, K2)                 \
template<K1 k1>                             \
struct C##Name {                            \
	template<K2 k2>                         \
	using Curry = NameSpace::Name<k1, k2>;  \
}

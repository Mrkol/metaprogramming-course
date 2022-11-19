#include <reflect.hpp>
#include <type_traits>
#include "annotations.hpp"
#include <ranges>

#define MAKE_FIELD \
MPC_ANNOTATE(SerialId<SizeT<4>>, SerialId<SizeT<5>>, SerialId<SizeT<6>>) int MPC_CONCAT(field, __COUNTER__);

#define MAKE_2(x) x x
#define MAKE_4(x) MAKE_2(x) MAKE_2(x)
#define MAKE_16(x) MAKE_4(x) MAKE_4(x)
#define MAKE_32(x) MAKE_16(x) MAKE_16(x)
#define MAKE_64(x) MAKE_32(x) MAKE_32(x)
#define MAKE_128(x) MAKE_64(x) MAKE_64(x)
#define MAKE_256(x) MAKE_128(x) MAKE_128(x)
#define MAKE_512(x) MAKE_256(x) MAKE_256(x)

#define MAKE_FIELDS MAKE_512(MAKE_FIELD)

struct Chonk {
  MAKE_FIELDS
};

static_assert(Describe<Chonk>::Field<0>::has_annotation_class<SerialId<SizeT<4>>>);
static_assert(Describe<Chonk>::Field<256>::has_annotation_class<SerialId<SizeT<6>>>);
static_assert(Describe<Chonk>::Field<511>::has_annotation_template<SerialId>);

int main() {}

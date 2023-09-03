#include <reflect.hpp>
#include <type_traits>
#include "annotations.hpp"

#define MAKE_FIELD_NAME(c) MPC_CONCAT(field, c)

#define MAKE_FIELD \
MPC_ANNOTATE(SerialId<SizeT<4>>, SerialId<SizeT<5>>, SerialId<SizeT<6>>) int MAKE_FIELD_NAME(__COUNTER__);

#define MAKE_2 MAKE_FIELD MAKE_FIELD
#define MAKE_4 MAKE_2 MAKE_2
#define MAKE_8 MAKE_4 MAKE_4
#define MAKE_16 MAKE_8 MAKE_8
#define MAKE_32 MAKE_16 MAKE_16
#define MAKE_64 MAKE_32 MAKE_32
#define MAKE_128 MAKE_64 MAKE_64
#define MAKE_256 MAKE_128 MAKE_128

using namespace mpc::annotations;

struct Chonk {
  MAKE_256
};

static_assert(Describe<Chonk>::num_fields == 256);
static_assert(Describe<Chonk>::Field<0>::has_annotation_class<SerialId<SizeT<4>>>);
static_assert(Describe<Chonk>::Field<128>::has_annotation_class<SerialId<SizeT<6>>>);
static_assert(Describe<Chonk>::Field<255>::has_annotation_template<SerialId>);

int main() {}

#include <reflect.hpp>
#include <type_traits>
#include "annotations.hpp"

#include <gtest/gtest.h>


void checkEmpty() {
  using namespace mpc::annotations;

  struct Empty {};
  static_assert(Describe<Empty>::num_fields == 0);
}

void checkReflection() {
  using namespace mpc::annotations;

  struct S1 {
    int x;

    MPC_ANNOTATE(Transient, NoIo) char y;

    MPC_ANNOTATE(Transient)
    MPC_ANNOTATE(NoCompare)
    float z;

    bool u, v;

    MPC_ANNOTATE(NoIo, Adapt<A, B, C>)
    long long w;
  };

  static_assert(Describe<S1>::num_fields == 6);

  static_assert(std::is_same_v<Describe<S1>::Field<0>::Type, int>);
  static_assert(std::is_same_v<Describe<S1>::Field<1>::Type, char>);
  static_assert(std::is_same_v<Describe<S1>::Field<2>::Type, float>);
  static_assert(std::is_same_v<Describe<S1>::Field<3>::Type, bool>);
  static_assert(std::is_same_v<Describe<S1>::Field<4>::Type, bool>);
  static_assert(std::is_same_v<Describe<S1>::Field<5>::Type, long long>);

  static_assert(std::is_same_v<Describe<S1>::Field<0>::Annotations, Annotate<>>);
  static_assert(std::is_same_v<Describe<S1>::Field<1>::Annotations, Annotate<Transient, NoIo>>);
  static_assert(std::is_same_v<Describe<S1>::Field<2>::Annotations, Annotate<Transient, NoCompare>>);
  static_assert(std::is_same_v<Describe<S1>::Field<3>::Annotations, Annotate<>>);
  static_assert(std::is_same_v<Describe<S1>::Field<4>::Annotations, Annotate<>>);
  static_assert(std::is_same_v<Describe<S1>::Field<5>::Annotations, Annotate<NoIo, Adapt<A, B, C>>>);
}

void checkLookup() {
  using namespace mpc::annotations;

  struct S2 {
    MPC_ANNOTATE(Transient, NoIo) char y;

    MPC_ANNOTATE(Transient) MPC_ANNOTATE(NoCompare) float z;

    bool u;

    MPC_ANNOTATE(NoIo, Adapt<A, B, C>) long long w;
  };

  static_assert(Describe<S2>::Field<0>::has_annotation_class<Transient>);
  static_assert(Describe<S2>::Field<0>::has_annotation_class<NoIo>);
  static_assert(!Describe<S2>::Field<0>::has_annotation_class<NoCompare>);

  static_assert(Describe<S2>::Field<1>::has_annotation_class<Transient>);
  static_assert(!Describe<S2>::Field<1>::has_annotation_class<NoIo>);
  static_assert(Describe<S2>::Field<1>::has_annotation_class<NoCompare>);

  static_assert(!Describe<S2>::Field<2>::has_annotation_class<Transient>);
  static_assert(!Describe<S2>::Field<2>::has_annotation_template<SerialId>);

  static_assert(Describe<S2>::Field<3>::has_annotation_class<NoIo>);
  static_assert(Describe<S2>::Field<3>::has_annotation_class<Adapt<A, B, C>>);
  static_assert(!Describe<S2>::Field<3>::has_annotation_class<NoCompare>);

  static_assert(Describe<S2>::Field<3>::has_annotation_template<Adapt>);
  static_assert(!Describe<S2>::Field<3>::has_annotation_template<SerialId>);
}

void checkMatching() {
  using namespace mpc::annotations;

  struct S3 {
    MPC_ANNOTATE(SerialId<SizeT<1>>, SerialId<SizeT<2>>, SerialId<SizeT<3>>) char x;
    MPC_ANNOTATE(Adapt<A, B, C>, SerialId<SizeT<4>>) float y;
  };

  using XDescriptor = Describe<S3>::Field<0>;
  using YDescriptor = Describe<S3>::Field<1>;

  static_assert(CanFindAnnotationTemplate<XDescriptor, SerialId>);
  static_assert(!CanFindAnnotationTemplate<XDescriptor, Adapt>);

  static_assert(CanFindAnnotationTemplate<YDescriptor, SerialId>);
  static_assert(CanFindAnnotationTemplate<YDescriptor, Adapt>);

  constexpr int found_id = XDescriptor::FindAnnotation<SerialId>::id;
  static_assert(found_id == 1 || found_id == 2 || found_id == 3);

  static_assert(std::is_same_v<YDescriptor::FindAnnotation<Adapt>, Adapt<A, B, C>>);
  static_assert(std::is_same_v<YDescriptor::FindAnnotation<SerialId>, SerialId<SizeT<4>>>);
}

void checkUserDefined() {
  struct UserDefined {};
  struct S4 {
    UserDefined x;
  };

  static_assert(Describe<S4>::num_fields == 1);
  static_assert(std::is_same_v<Describe<S4>::Field<0>::Type, UserDefined>);
}

void checkMultiple() {
  using namespace mpc::annotations;

  struct S5 {
    MPC_ANNOTATE(SerialId<SizeT<0>>, SerialId<SizeT<1>>)
    MPC_ANNOTATE(SerialId<SizeT<1>>, SerialId<SizeT<2>>)
    MPC_ANNOTATE(SerialId<SizeT<2>>, SerialId<SizeT<3>>)
    MPC_ANNOTATE(SerialId<SizeT<3>>, SerialId<SizeT<4>>)
    MPC_ANNOTATE(SerialId<SizeT<4>>, SerialId<SizeT<5>>)
    int x;
  };

  static_assert(Describe<S5>::num_fields == 1);
  static_assert(std::is_same_v<Describe<S5>::Field<0>::Type, int>);

  using ExpectedAnnotations = Annotate<
    SerialId<SizeT<0>>,
    SerialId<SizeT<1>>, SerialId<SizeT<1>>,
    SerialId<SizeT<2>>, SerialId<SizeT<2>>,
    SerialId<SizeT<3>>, SerialId<SizeT<3>>,
    SerialId<SizeT<4>>, SerialId<SizeT<4>>,
    SerialId<SizeT<5>>
  >;
  static_assert(std::is_same_v<Describe<S5>::Field<0>::Annotations, ExpectedAnnotations>);

  static_assert(Describe<S5>::Field<0>::has_annotation_class<SerialId<SizeT<0>>>);
  static_assert(Describe<S5>::Field<0>::has_annotation_class<SerialId<SizeT<1>>>);
  static_assert(Describe<S5>::Field<0>::has_annotation_class<SerialId<SizeT<2>>>);
  static_assert(Describe<S5>::Field<0>::has_annotation_class<SerialId<SizeT<3>>>);
  static_assert(Describe<S5>::Field<0>::has_annotation_class<SerialId<SizeT<4>>>);
  static_assert(Describe<S5>::Field<0>::has_annotation_class<SerialId<SizeT<5>>>);
  static_assert(Describe<S5>::Field<0>::has_annotation_class<SerialId<SizeT<0>>>);
  static_assert(Describe<S5>::Field<0>::has_annotation_template<SerialId>);
}


TEST(AnnotationsTest, JustWorks)
{
  checkEmpty();
  checkReflection();
  checkLookup();
  checkMatching();
  checkUserDefined();
  checkMultiple();
}

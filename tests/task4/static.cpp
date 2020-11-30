#include <reflect.hpp>
#include <type_traits>


#define mpg_concat(a,b)  a##b
#define mpg_annotation_label(a) mpg_concat(_annotion_, a)
#define mpg_annotate(...) Annotate<__VA_ARGS__> mpg_annotation_label(__COUNTER__);


namespace mpg::annotations {

struct Transient;
struct NoIo;
struct NoCompare;

struct A;
struct B;
struct C;

template <class... Adaptors>
struct Adapt;

template <size_t n>
using SizeT = std::integral_constant<size_t, n>;

template <class>
struct SerialId;

template <class>
struct SerialId;

template <size_t n>
struct SerialId<SizeT<n>> {
    static constexpr size_t id = n;
};

template <class FieldDescriptor, template <class...> class AnnotationTemplate>
concept CanFindAnnotationTemplate = requires {
    typename FieldDescriptor::template FindAnnotation<AnnotationTemplate>;
};

}

void checkEmpty() {
    using namespace mpg::annotations;

    struct Empty {};
    static_assert(Describe<Empty>::num_fields == 0);
}

void checkReflection() {
    using namespace mpg::annotations;

    struct S1 {
        int x;

        mpg_annotate(Transient, NoIo) char y;

        mpg_annotate(Transient)
        mpg_annotate(NoCompare)
        float z;

        bool u, v;

        mpg_annotate(NoIo, Adapt<A, B, C>)
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
    using namespace mpg::annotations;

    struct S2 {
        mpg_annotate(Transient, NoIo) char y;

        mpg_annotate(Transient) mpg_annotate(NoCompare) float z;

        bool u;

        mpg_annotate(NoIo, Adapt<A, B, C>) long long w;
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
    using namespace mpg::annotations;

    struct S3 {
        mpg_annotate(SerialId<SizeT<1>>, SerialId<SizeT<2>>, SerialId<SizeT<3>>) char x;
        mpg_annotate(Adapt<A, B, C>, SerialId<SizeT<4>>) float y;
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

int main() {
    checkEmpty();
    checkReflection();
    checkLookup();
    checkMatching();
    return 0;
}

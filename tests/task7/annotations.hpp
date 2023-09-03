#pragma once

#include <cstdint>


#define MPC_CONCAT(a,b)  a##b
#define MPC_ANNOTATION_LABEL(a) MPC_CONCAT(_annotion_, a)
#define MPC_ANNOTATE(...) Annotate<__VA_ARGS__> MPC_ANNOTATION_LABEL(__COUNTER__);


namespace mpc::annotations {

struct Transient;
struct NoIo;
struct NoCompare;

struct A;
struct B;
struct C;

template <class... Adaptors>
struct Adapt;

template <std::size_t n>
using SizeT = std::integral_constant<std::size_t, n>;

template <class>
struct SerialId;

template <class>
struct SerialId;

template <std::size_t n>
struct SerialId<SizeT<n>> {
  static constexpr std::size_t id = n;
};

template <class FieldDescriptor, template <class...> class AnnotationTemplate>
concept CanFindAnnotationTemplate = requires {
  typename FieldDescriptor::template FindAnnotation<AnnotationTemplate>;
};

}

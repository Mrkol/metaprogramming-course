#pragma once
#include "tag_invoke.hpp"

#include <iostream>

namespace shapes
{
  inline constexpr struct width_t {
    using type_erased_signature_t = float(const mpc::this_&);

    template<typename T>
      requires mpc::tag_invocable<width_t, const T&>
    float operator()(const T& x) const {
      return mpc::tag_invoke(*this, x);
    }
  } width;

  inline constexpr struct height_t {
    using type_erased_signature_t = float(const mpc::this_&);

    template<typename T>
      requires mpc::tag_invocable<height_t, const T&>
    float operator()(const T& x) const {
      return mpc::tag_invoke(*this, x);
    }
  } height;

  inline constexpr struct area_t {
    using type_erased_signature_t = float(const mpc::this_&);

    template<typename T>
      requires mpc::tag_invocable<area_t, const T&>
    float operator()(const T& x) const {
      return mpc::tag_invoke(*this, x);
    }
  } area;

  inline constexpr struct scale_by_t {
    template<typename T>
      requires mpc::tag_invocable<scale_by_t, const T&, float>
    auto operator()(const T& x, float ratio) const
        -> mpc::tag_invoke_result_t<scale_by_t, const T&, float> {
      return mpc::tag_invoke(*this, x, ratio);
    }
  } scale_by;

  template<typename T>
  concept shape =
    std::copyable<T> &&
    requires(const T& s, float ratio) {
      shapes::width(s);
      shapes::height(s);
      shapes::area(s);
      { shapes::scale_by(s, ratio) } -> std::same_as<T>;
    };

//////////////////////////////////////////////////////////////////

  struct square {
    float size;
  public:
    explicit square(float size) : size(size) {}

    friend float tag_invoke(mpc::tag_t<shapes::width>, const square& s) {
      return s.size;
    }

    friend float tag_invoke(mpc::tag_t<shapes::height>, const square& s) {
      return s.size;
    }

    friend float tag_invoke(mpc::tag_t<shapes::area>, const square& s) {
      std::cout << "area";
      return s.size * s.size;
    }

    friend square tag_invoke(mpc::tag_t<shapes::scale_by>, const square& s, float ratio) {
      std::cout << "scale_by";
      return square{s.size * ratio};
    }
  };

  using any_shape1 = any_object<
    true,
    mpc::tag_t<shapes::width>,
    mpc::tag_t<shapes::height>,
    mpc::tag_t<shapes::area>>;

  // square sq;
  // any_shape1 a = sq;
  // shapes::width(a); -> width(sq)


  // using any_shape2 =  mpc::any_object<
  //   shapes::width,
  //   shapes::height,
  //   shapes::area,
  //   mpc::overload<any_shape2(const mpc::this_&, float)>(shapes::scale_by)>;

  class any_shape : public any_object<
    true,
    mpc::tag_t<shapes::width>,
    mpc::tag_t<shapes::height>,
    mpc::tag_t<shapes::area>,
    mpc::tag_t<mpc::overload<any_shape(const mpc::this_&, float)>(shapes::scale_by)>>
  {
  private:
    using base_t = any_object<
      true,
      mpc::tag_t<shapes::width>,
      mpc::tag_t<shapes::height>,
      mpc::tag_t<shapes::area>,
      mpc::tag_t<mpc::overload<any_shape(const mpc::this_&, float)>(shapes::scale_by)>>;
  public:
    // Inherit the constructors
    using base_t::base_t;
  };

}

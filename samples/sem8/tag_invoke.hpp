#pragma once

#include <concepts>
#include <utility>


namespace mpc {
  namespace _tag_invoke {
    void tag_invoke() = delete;

    struct tag_invoke_fn {
      template<class CPO, class... Args>
      constexpr auto operator()(CPO cpo, Args&&... args) const
        noexcept(noexcept(tag_invoke(cpo, (Args&&)args...)))
        -> decltype(tag_invoke(cpo, (Args&&)args...))  /* SFINAE-friendliness? */ {
        return tag_invoke(cpo, (Args&&)args...);
      }
    };
  }

  inline namespace _cpo {
    inline constexpr _tag_invoke::tag_invoke_fn tag_invoke;
  }

  template<auto& Tag>
  using tag_t = std::decay_t<decltype(Tag)>;

  template<class Tag, class... Args>
  concept tag_invocable = std::invocable<decltype(tag_invoke), Tag, Args...>;

  template<class Tag, class... Args>
  concept nothrow_tag_invocable = tag_invocable<Tag, Args...> &&
    std::is_nothrow_invocable_v<decltype(tag_invoke), Tag, Args...>;

  template<class Tag, class... Args>
  using tag_invoke_result = std::invoke_result<decltype(tag_invoke), Tag, Args...>;

  template<class Tag, class... Args>
  using tag_invoke_result_t = typename tag_invoke_result<Tag, Args...>::type;


  struct this_ {};


  namespace detail {

  template <auto CPO, typename Signature>
  struct overloaded_cpo;

  }; // namespace detail


  template <class Signature, class CPO>
  struct OverloadedCpo {
    OverloadedCpo() = default;
    OverloadedCpo(CPO) {}

    using type_erased_signature_t = Signature;

    template <typename... Args>
    decltype(auto) operator()(Args&&... args) {
      return mpc::tag_invoke(CPO{}, std::forward<Args>(args)...);
    }
  };
  template <class Signature, class CPO>
  constexpr OverloadedCpo<Signature, CPO> overloaded_storage{};

  template <class Signature, class CPO>
  constexpr auto& overload(CPO) {
    // return detail::overloaded_cpo<CPO, Signature>{};

    return overloaded_storage<Signature, CPO>;
  };
} // namespace mpc

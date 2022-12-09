#pragma once

#include <concepts>
#include <array>
#include <stdexcept>
#include <utility>

template <typename T>
concept DependentFalse = false;

inline constexpr std::size_t SMALL_BUFFER_SIZE = 40;
inline constexpr std::size_t ALIGNMENT = 64;

union Object {
  void* dynObject_;
  alignas(ALIGNMENT) std::array<std::byte, SMALL_BUFFER_SIZE> staticObject_;
};


template<typename From, typename To, typename T>
struct ReplaceOne {
  using type = T;
};

template<typename From, typename To>
struct ReplaceOne<From, To, From> {
  using type = To;
};
template<typename From, typename To>
struct ReplaceOne<From, To, const From> {
  using type = const To;
};
template<typename From, typename To>
struct ReplaceOne<From, To, From&> {
  using type = To&;
};
template<typename From, typename To>
struct ReplaceOne<From, To, const From&> {
  using type = const To&;
};
template<typename From, typename To>
struct ReplaceOne<From, To, From&&> {
  using type = To&&;
};

template<class From, class To, class Signature>
struct ReplaceThisImpl;

template<class From, class To, class T, class... Us>
struct ReplaceThisImpl<From, To, T(Us...)> {
  using Type = typename ReplaceOne<From, To, T>::type(
    typename ReplaceOne<From, To, Us>::type...);
};

template<class CPO, class Signature>
struct TagInvocableImpl;

template<class CPO, class T, class... Us>
struct TagInvocableImpl<CPO, T(Us...)> {
  static constexpr bool value = mpc::tag_invocable<CPO, Us...>;
};

template<class CPO, class T, class SigErased>
struct DoTagInvoke;

template<class CPO, class T, class RetErased, class... ArgsErased>
struct DoTagInvoke<CPO, T, RetErased(ArgsErased...)> {
  static auto make_tag_invoke() {
    static constexpr auto prepare_arg = [](auto&& arg) -> decltype(auto) {
      if constexpr (std::is_same_v<std::remove_cvref_t<decltype(arg)>, Object>) {
        using Tptr = std::conditional_t<
          std::is_const_v<std::remove_reference_t<decltype(arg)>>,
          const T*, T*>;
        if constexpr (sizeof(T) < SMALL_BUFFER_SIZE)
          return *std::launder(reinterpret_cast<Tptr>(arg.staticObject_.data()));
        else
          return *static_cast<Tptr>(arg.dynObject_);
      }
      else {
        return std::forward<decltype(arg)>(arg);
      }
    };
    return +[](ArgsErased... args) -> RetErased {
      static_assert(!std::is_same_v<std::remove_cvref_t<RetErased>, Object>);
      return mpc::tag_invoke(CPO{}, prepare_arg(std::forward<ArgsErased>(args))...);
    };
  }
};



template<class CPO>
struct vtable_entry
{
  using ModifiedSignature = typename ReplaceThisImpl<mpc::this_, Object, typename CPO::type_erased_signature_t>::Type;
  // DONE: replace this_ with Object&
  ModifiedSignature* fptr = nullptr;

  template<class T>
  //  requires mpc::tag_invocable<CPO, ..., T, ...>
    // requires TagInvocableImpl<CPO, ModifiedSignature>::value
  static vtable_entry create()
  {
    return vtable_entry{
      .fptr = DoTagInvoke<CPO, T, ModifiedSignature>::make_tag_invoke()
    };
  };


  void setImpl(CPO, ModifiedSignature f) {
    fptr = f;
  }
  auto getImpl(CPO) const {
    return fptr;
  }
};

template<class... CPOs>
struct vtable : vtable_entry<CPOs>...
{
  using vtable_entry<CPOs>::getImpl...;
  using vtable_entry<CPOs>::setImpl...;

  template<class CPO>
  auto get() const
  {
    return getImpl(CPO{});
  }

  template<class T>
  static vtable create()
  {
    vtable object;
    ((object.setImpl(CPOs{}, vtable_entry<CPOs>::template create<T>().fptr)), ...);
    return object;
  };
};

template<class... CPOs>
struct inline_vtable_storage : vtable<CPOs...> {
  inline_vtable_storage() = default;

  template<class T>
  inline_vtable_storage(const T&)
    : vtable<CPOs...>{vtable<CPOs...>::template create<T>()} {
  }

  using vtable<CPOs...>::get;
};

template <class T, class... CPOs>
struct class_specific_vtable_storage
{
  static vtable<CPOs...> * get_vtable() {
    static vtable<CPOs...> static_vtable = vtable<CPOs...>::create();
    return &static_vtable;
  }
};

template <class... CPOs>
struct indirect_vtable_storage
{

  template <class T>
  indirect_vtable_storage(const T&)
    : vt(class_specific_vtable_storage<T>::get_vtable()) {
  }

  indirect_vtable_storage() = default;

  template <class CPO>
  auto get() const {
    return vt->template get<CPO>();
  }

  vtable<CPOs...> *vt = nullptr; // ptr to **static** storage
};

template<bool inlineVtable, class... CPOs>
class any_object
{
  using vtable_t = vtable<CPOs...>;
public:
  any_object() = default;

  template<class T>
    requires (!std::derived_from<std::decay_t<T>, any_object>
      && !std::same_as<std::decay_t<T>, any_object>
      && std::copyable<std::remove_cvref_t<T>>)
  any_object(T&& t)
    : vt_{t} {
    using Value = std::remove_cvref_t<T>;

    static_assert(alignof(Value) <= ALIGNMENT);

    if constexpr (sizeof(Value) <= SMALL_BUFFER_SIZE)
      new(object_.staticObject_.data()) Value(std::forward<T>(t));
    else
      object_.dynObject_ = new Value(std::forward<T>(t));

    destructor_ = &destroy<Value>;
    copy_ = &copy<Value>;
    move_ = &move<Value>;
  }

  any_object(any_object&& other) {
    move_(object_, other.object_);
    destructor_ = other.destructor_;
    copy_ = other.copy_;
    move_ = other.move_;
    vt_ = other.vt_;
  }

  any_object& operator=(any_object&& other) {
    if (this != &other) {
      clear();

      move_(object_, other.object_);
      destructor_ = std::exchange(other.destructor_, nullptr);
      copy_ = std::exchange(other.copy_, nullptr);
      move_ = std::exchange(other.move_, nullptr);
      vt_ = std::exchange(other.vt_, decltype(other.vt_){});
    }

    return *this;
  }

  any_object(const any_object& other) {
    destructor_ = other.destructor_;
    copy_ = other.copy_;
    vt_ = other.vt_;
    copy_(object_, other.object_);
  }

  any_object& operator=(const any_object& other) {
    if (this != &other) {
      clear();
      destructor_ = other.destructor_;
      copy_ = other.copy_;
      vt_ = other.vt_;
      copy_(object_, other.object_);
    }
    return *this;
  }

  ~any_object() {
    clear();
  }

  void clear() {
    if (destructor_)
    {
      destructor_(object_);
      copy_ = nullptr;
      destructor_ = nullptr;
      move_ = nullptr;
      vt_ = {};
    }
  }

private:
  template<class T>
  friend T& any_cast(any_object& a) {
    if (a.destructor_ == &any_object::destroy<T>)
    {
      if constexpr (sizeof(T) <= SMALL_BUFFER_SIZE)
        return *std::launder(reinterpret_cast<T*>(a.object_.staticObject_.data()));
      else
        return *static_cast<T*>(&a.object_.dynObject_);
    }
    else
      throw std::runtime_error("Bad any_object cast!");
  }

  template<class T>
  static void destroy(Object& ptr)
  {
    if constexpr (sizeof(T) < SMALL_BUFFER_SIZE)
      std::launder(reinterpret_cast<T*>(ptr.staticObject_.data()))->~T();
    else
      delete static_cast<T*>(ptr.dynObject_);
  }

  template<class T>
  static void copy(Object& to, const Object& from)
  {
    if constexpr (sizeof(T) < SMALL_BUFFER_SIZE)
      new(to.staticObject_.data()) T(
        *std::launder(reinterpret_cast<const T*>(from.staticObject_.data())));
    else
      to.dynObject_ = new T(*static_cast<T*>(from.dynObject_));
  }

  template<class T>
  static void move(Object& to, Object& from)
  {
    if constexpr (sizeof(T) < SMALL_BUFFER_SIZE)
    {
      auto* object = std::launder(reinterpret_cast<T*>(from.staticObject_.data()));
      new(to.staticObject_.data()) T(std::move(*object));
      object->~T();
      from.dynObject_ = nullptr;
    }
    else
      to.dynObject_ = std::exchange(from.dynObject_, nullptr);
  }

  // TODO: tag_invoke(CPOs)...

  operator Object& () { return object_; }
  operator const Object& () const { return object_; }

  template <typename CPO, typename... Args>
  friend decltype(auto) tag_invoke(CPO, Args&&... args)
    requires requires(std::tuple<CPOs...> t) { std::get<CPO>(t); } {
    (([&]<class T>(T&& arg) mutable {
      if constexpr (std::derived_from<std::decay_t<T>, any_object>) {
        auto func = arg.vt_.template get<CPO>();
        // static_assert(DependentFalse<decltype(&func)>);
        func(std::forward<Args>(args)...);
      }
    }(args)), ...);
  }

  // TODO: cross-cast to any_object with a (non-strict) subset of CPOs?

private:
  Object object_{nullptr};

  // TODO: replace with something CPO-ish?
  void(*copy_)(Object&, const Object&) {nullptr};
  void(*move_)(Object&, Object&) {nullptr};
  void(*destructor_)(Object&) {nullptr};

  // TODO: store and use CPO vtable
  std::conditional_t<inlineVtable,
    inline_vtable_storage<CPOs...>, indirect_vtable_storage<CPOs...>>
    vt_;
};

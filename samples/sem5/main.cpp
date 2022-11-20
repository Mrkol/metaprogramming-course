#include <iostream>
#include <memory>
#include <concepts>
#include <utility>
#include <array>
#include <variant>
#include <any>


class Any
{
public:
  static constexpr size_t SMALL_BUFFER_SIZE = 40;
  static constexpr size_t ALIGNMENT = 64;

  Any() = default;

  template<class T>
  Any(T&& t) requires std::regular<T> {
    static_assert(alignof(T) <= ALIGNMENT);

    if constexpr (sizeof(T) <= SMALL_BUFFER_SIZE)
      new(object_.staticObject_.data()) T(std::forward<T>(t));
    else
      object_.dynObject_ = new T(std::forward<T>(t));

    destructor_ = &destroy<T>;
    copy_ = &copy<T>;
    move_ = &move<T>;
  }

  Any(Any&& other) {
    move_(object_, other.object_);
    destructor_ = other.destructor_;
    copy_ = other.copy_;
    move_ = other.move_;
  }

  Any& operator=(Any&& other) {
    if (this != &other) {
      clear();

      move_(object_, other.object_);
      destructor_ = std::exchange(other.destructor_, nullptr);
      copy_ = std::exchange(other.copy_, nullptr);
      move_ = std::exchange(other.move_, nullptr);
    }

    return *this;
  }

  Any(const Any& other) {
    destructor_ = other.destructor_;
    copy_ = other.copy_;
    copy_(object_, other.object_);
  }

  Any& operator=(const Any& other) {
    if (this != &other) {
      clear();
      destructor_ = other.destructor_;
      copy_ = other.copy_;
      copy_(object_, other.object_);
    }
    return *this;
  }

  ~Any() {
    clear();
  }

  void clear() {
    if (destructor_)
    {
      destructor_(object_);
      copy_ = nullptr;
      destructor_ = nullptr;
      move_ = nullptr;
    }
  }

private:
  union Object {
    void* dynObject_;
    alignas(ALIGNMENT) std::array<std::byte, SMALL_BUFFER_SIZE> staticObject_;
  };

  template<class T>
  friend T& any_cast(Any& a) {
    if (a.destructor_ == &Any::destroy<T>)
    {
      if constexpr (sizeof(T) <= SMALL_BUFFER_SIZE)
        return *std::launder(reinterpret_cast<T*>(a.object_.staticObject_.data()));
      else
        return *static_cast<T*>(&a.object_.dynObject_);
    }
    else
      throw std::runtime_error("Bad any cast!");
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

private:
  Object object_{nullptr};

  struct VTable
  {
    void(*copy_)(Object&, const Object&) {nullptr};
    void(*move_)(Object&, Object&) {nullptr};
    void(*destructor_)(Object&) {nullptr};
  };

  VTable* vtable;
};

int main() {
  Any a = 42;

  std::cout << any_cast<int>(a) << std::endl;

  using std::begin;
  begin(container);


  Any b = std::string("Hello!");
  a = b;

  std::cout << any_cast<std::string>(a) << std::endl;

  return 0;
}

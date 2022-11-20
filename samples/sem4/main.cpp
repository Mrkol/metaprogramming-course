#include <iostream>
#include <memory>
#include <concepts>
#include <utility>


template<class Sig>
class Function;

template<class F, class Ret, class... Args>
class Function<Ret(F::*)(Args...)>
  : public Function<Ret(Args...)> {
  using Function<Ret(Args...)>::Function;
};

template<class Ret, class... Args>
class Function<Ret(Args...)> {
  struct ICallable {
    virtual Ret call(Args...) = 0;
    virtual ~ICallable() = default;
    virtual std::unique_ptr<ICallable> clone() = 0;
  };

  template<class F>
  struct ConcreteCallable : ICallable {
    ConcreteCallable(F f) : func{std::move(f)} {}

    Ret call(Args... args) override {
      return func(std::forward<Args>(args)...);
    }

    std::unique_ptr<ICallable> clone() override {
      return std::make_unique<ConcreteCallable>(func);
    }

    F func;
  };
public:
  template<std::invocable<Args...> F>
  Function(F f)
    : impl_{std::make_unique<ConcreteCallable<F>>(std::move(f))}
  {

  }

  Function(Function const & other)
    : impl_{other.impl_->clone()} {
  }

  Function& operator=(Function const & other) {
    if (&other != this) {
      impl_ = other.impl_->clone();
    }

    return *this;
  }

  Ret operator()(Args... args) {
    return impl_->call(std::forward<Args>(args)...);
  }

private:
  std::unique_ptr<ICallable> impl_;
};

void foo(Function<void(int)> f) {
  f(0);
}

template<class F>
Function(F f) -> Function<decltype(&F::operator())>;

template<class Ret, class... Args>
Function(Ret (*f)(Args...)) -> Function<Ret(Args...)>;

struct Foo {};
using FuncType = void(int, float) const;
using MethodPtrType = FuncType (Foo::*);

struct FunctionalObject {
  void operator()(int) {
    std::cout << "Hello from struct!";
  };
};

void banana(int) {}

int main() {
  Function f(
    [called_once = false](int) mutable {
      if (!std::exchange(called_once, true))
        std::cout << "Hello!";
    });

  Function<void(int)> g(FunctionalObject{});
  g(0);

  foo(f);
  foo(f);
  return 0;
}

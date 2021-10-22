#include <memory>
#include <functional>
#include <concepts>
#include <variant>
#include <iostream>
#include <any>

// CRTP = curiously recurring template pattern

template<class Derived, class Base>
struct Cloneable : Base
{
    Base* clone() override { return new Derived(*static_cast<Derived*>(this)); }
};

struct Parent
{
    virtual Parent* clone() { return new Parent(*this); }
};

struct Child : Cloneable<Child, Parent>
{
};


template<class Derived>
struct Comparable
{
    [[nodiscard]] const Derived& self() const { return *static_cast<Derived*>(this); }

    bool operator>=(const Derived& d) { return d <= self(); }
    bool operator==(const Derived& d) { return self() >= d && d >= self(); }
};

struct LongInt : Comparable<LongInt>
{
    bool operator<=(const LongInt&);
};


namespace A
{

struct AnyImpl {
    virtual std::intptr_t get_id() = 0;
    virtual std::unique_ptr<AnyImpl> clone() = 0;
    virtual ~AnyImpl() {}
};

template<std::semiregular T>
struct ConcreteImpl : AnyImpl {
    ConcreteImpl(T t_) : t{std::move(t_)} {}

    static void hack(T*) {};

    static std::intptr_t id() { return reinterpret_cast<std::intptr_t>(&hack); }

    std::intptr_t get_id() override
    {
        return id();
    }

    std::unique_ptr<AnyImpl> clone() override
    {
        return std::unique_ptr<AnyImpl>(new ConcreteImpl(t));
    }

    T t;
};

struct Any
{
    template<typename T>
        requires (!std::same_as<std::remove_cvref_t<T>, Any> && std::semiregular<std::remove_cvref_t<T>>)
    Any(T&& t)
        : impl_{new ConcreteImpl<std::remove_cvref_t<T>>{std::forward<T>(t)}} {

    }

    Any(const Any& other)
            : impl_{other.impl_->clone()} {
    }

    Any(Any&& other)
            : impl_{std::move(other.impl_)} {
    }

    Any& operator=(const Any& other) {
        if (this == &other)
        {
            return *this;
        }

        impl_ = other.impl_->clone();

        return *this;
    }

    Any& operator=(Any&& other) {
        if (this == &other)
        {
            return *this;
        }

        impl_ = std::move(other.impl_);

        return *this;
    }

    template<typename T>
    T& get() & {
        if (impl_->get_id() == ConcreteImpl<T>::id())
        {
            return static_cast<ConcreteImpl<T>*>(impl_.get())->t;
        }
        else
        {
            throw std::bad_any_cast();
        }
    }

    template<typename T>
    const T& get() const & {
        if (impl_->get_id() == ConcreteImpl<T>::id())
        {
            return static_cast<ConcreteImpl<T>*>(impl_.get())->t;
        }
        else
        {
            throw std::bad_any_cast();
        }
    }

    template<typename T>
    T&& get() && {
        if (impl_->get_id() == ConcreteImpl<T>::id())
        {
            return std::move(static_cast<ConcreteImpl<T>*>(impl_.get())->t);
        }
        else
        {
            throw std::bad_any_cast();
        }
    }

    std::unique_ptr<AnyImpl> impl_;
};


template<class T, std::common_reference_with<Any> A>
auto any_cast(A&& a) noexcept( noexcept(std::forward<A>(a).template get<T>()) )
    -> decltype(std::forward<A>(a).template get<T>())
{
    return std::forward<A>(a).template get<T>();
}

}


namespace F1
{

template<class R, class... Args>
struct Function
{
    struct FunctionImpl {
        virtual R call_cl(Args... args) const & = 0;
        virtual R call_l(Args... args) & = 0;
        virtual R call_r(Args... args) && = 0;
        virtual ~FunctionImpl() {}
    };

    template<std::invocable<Args...> F>
    struct InvocableImpl : FunctionImpl {
        InvocableImpl(F f_) : f{std::move(f_)} {}

        R call_cl(Args... args) const & override {
            return std::invoke(f, args...);
        }

        R call_l(Args... args) & override {
            return std::invoke(f, args...);
        }

        R call_r(Args... args) && override {
            return std::invoke(std::move(f), args...);
        }

        F f;
    };

    template<typename C>
    struct MemberFunctionImpl : FunctionImpl {
        MemberFunctionImpl(C* c, R (C::*method)(Args...)) : c(c), method(method) {};

        R call(Args... args) override {
            return (c->*method)(args...);
        }

        C* c;
        R (C::*method)(Args...);
    };

    template<std::invocable<Args...> F>
    Function(F&& f) : impl_(new InvocableImpl<F>(std::forward<F>(f))) {}

    template<class C>
    Function(C* c, R (C::*method)(Args...))
        : impl_(new MemberFunctionImpl<C>(c, method))
    {}

    R operator()(Args... args) & {
        return impl_->call_l(args...);
    }

    R operator()(Args... args) const & {
        return impl_->call_cl(args...);
    }

    R operator()(Args... args) && {
        return std::move(*impl_).call_r(args...);
    }


    std::unique_ptr<FunctionImpl> impl_;
};


struct BadFO
{
    int operator()() &&
    {
        return 42;
    }
    int operator()() &
    {
        return 43;
    }
    int operator()() const &
    {
        return 44;
    }
};

}


template<class F>
class Function;


template<class F, class R, class... Args>
R call(void* self, Args... args)
{
    return static_cast<F*>(self)->operator()(args...);
}

struct alignas(4*sizeof(float)) Vec4
{
    float data[4];
};


struct InvokTable
{
    void(*dtor_)(void*) {nullptr};
    void*(*copy_)(void*) {nullptr};
};

template<class R, class... Args>
class Function<R(Args...)>
{
    using FuncPtr = R(*)(void*, Args...);

public:
    constexpr static std::size_t SMALL_BUFFER_SIZE = 64;
    constexpr static std::size_t ALIGN = 16;

    template<class F>
            requires (!std::same_as<std::remove_cvref_t<F>, Function>)
    Function(F&& f)
    {
        using Func = std::remove_cvref_t<F>;
        if constexpr (sizeof(Func) <  SMALL_BUFFER_SIZE && alignof(Func) <= ALIGN)
        {
            data_.emplace<1>();
            new(&std::get<1>(data_)) Func(std::forward<F>(f));
        }
        else
        {
            data_ = new F(std::forward<F>(f));

            call_ = &(call<Func, R, Args...>);
            dtor_ = +[](void* self) { delete static_cast<Func*>(self); };
            copy_ = +[](void* self) -> void*
                {
                    return new Func(*static_cast<Func*>(self));
                };
        }
    }

    Function(Function& other)
    {
        data_ = other.copy_(other.data_);

        call_ = other.call_;
        dtor_ = other.dtor_;
        copy_ = other.copy_;
    }

    Function& operator=(Function& other)
    {
        if (dtor_ != nullptr)
        {
            dtor_(data_);
        }
        data_ = other.copy_(other.data_);

        call_ = other.call_;
        dtor_ = other.dtor_;
        copy_ = other.copy_;
    }

    R operator()(Args... args)
    {
        return call_(data_, args...);
    }

    ~Function()
    {
        dtor_(data_);
    }

private:
    FuncPtr call_{nullptr};

    InvokTable table;
    std::variant<std::monostate, std::aligned_storage<SMALL_BUFFER_SIZE, ALIGN>, void*> data_;
};


struct Tracer
{
    Tracer() { std::cout << "ctor\n"; }

    Tracer(const Tracer&) { std::cout << "copy ctor\n"; }
    Tracer(Tracer&&) noexcept { std::cout << "move ctor\n"; }

    int operator()(int i)
    {
        return 2 + i;
    }

    Tracer& operator=(const Tracer&) { std::cout << "copy\n"; return *this; }
    Tracer& operator=(Tracer&&) noexcept { std::cout << "move\n";  return *this;}

    ~Tracer() { std::cout << "dtor\n"; }
};


int main()
{
    Function<int(int)> f(Tracer{});
    std::cout << f(2);

    return 0;
}

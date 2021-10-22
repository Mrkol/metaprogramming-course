#include <concepts>
#include <type_traits>
#include <iostream>
#include <array>
#include <ranges>
#include <vector>

template<size_t max_length>
struct StaticString {
    constexpr StaticString(const char* start, size_t length)
            : size(length) {
        std::copy(start, start + length, data.begin());
    }

    constexpr operator std::string_view() const {
        return std::string_view(data.data(), size);
    }

    size_t size {0};
    std::array<char, max_length> data;
};

inline constexpr StaticString<64> operator ""_cstr(const char* start, size_t length) {
    return {start, length};
}

template<size_t size>
std::ostream& operator<<(std::ostream& out, StaticString<size> str) {
    return out << std::string_view(str);
}






template<class Key, Key key, class F>
struct vtable_entry;


template<auto a>
struct ValueTag{};

template<class Key, Key key, class Ret, class... Args>
struct vtable_entry<Key, key, Ret(Args...)>
{
    using fn_t = Ret(*)(Args...);

    vtable_entry* cast(ValueTag<key>) { return this; }

    fn_t value;
};

template<auto K, class V>
struct KeyValue {
    constexpr static auto Key = K;
    using Value = V;
};

template<class Key, class... KV>
struct vtable;

template<class Key, Key... keys, class... Values>
struct vtable<Key, KeyValue<keys, Values>...> : vtable_entry<Key, keys, Values>...
{
    using vtable_entry<Key, keys, Values>::cast...;

    vtable(std::decay_t<Values>... vals)
        : vtable_entry<Key, keys, Values>{vals}...
    {

    }

    vtable()
        : vtable_entry<Key, keys, Values>{nullptr}...
    {

    }


    template<Key key>
    auto get()
    {
        return cast(ValueTag<key>{})->value;
    }

private:
};

namespace OOP
{

/*
 * [ vtable* | int a ]
 */
struct IBase
{
    virtual void foo() = 0;
    virtual void bar() = 0;

    int a = 42;
};

/*
 * Derived1 [ IBase [vtable* | int a] | b ]
 */
struct Derived1 : IBase
{
    void foo() override { std::cout << "foo1\n"; }
    void bar() override { std::cout << "bar1\n"; }
    int b;
};

struct Derived2 : IBase
{
    void foo() override { std::cout << "foo2\n"; }
    void bar() override { std::cout << "bar2\n"; }
};

// (void foo(int))
// table2 (void foo(int), void bar(bool))
// table3 (void foo(int), void bar(bool), void baz(float))

// table2* tbl = ...;

template<class... KVs>
using oop_vtable = vtable<StaticString<64>, KVs...>;


using cool_vtable = oop_vtable<KeyValue<"foo"_cstr, void()>, KeyValue<"bar"_cstr, void()>>;


void baz(IBase* base)
{
    base->foo();
}

void foo() { std::cout << "foo!\n"; };
void bar() {};





struct ICoolBase
{
    using vtable_t = oop_vtable<KeyValue<"foo"_cstr, void(ICoolBase*)>, KeyValue<"bar"_cstr, void(ICoolBase*)>>;

    template<StaticString<64> s, class... Args>
    decltype(auto) call_virtual(ValueTag<s>, Args&&... args)
    {
        return (*vtbl->get<s>())(this, std::forward<Args>(args)...);
    }

    inline static vtable_t holder{};
    vtable_t* vtbl = &holder;
    int a = 42;
};

/*
 * Derived1 [ IBase [vtable* | int a] | b ]
 */
struct CoolDerived1 : ICoolBase
{
    CoolDerived1()
    {
        vtbl = &holder;
    }

    ~CoolDerived1()
    {
        vtbl = &ICoolBase::holder;
    }

    void foo() { std::cout << "foo1\n"; }
    void bar() { std::cout << "bar1\n"; }

    inline static vtable_t holder{
            +[](ICoolBase* base) { static_cast<CoolDerived1*>(base)->foo(); },
            +[](ICoolBase* base) { static_cast<CoolDerived1*>(base)->bar(); }
        };
    int b;
};

struct CoolDerived2 : ICoolBase
{
    CoolDerived2()
    {
        vtbl = &holder;
    }

    ~CoolDerived2()
    {
        vtbl = &ICoolBase::holder;
    }

    void foo() { std::cout << "foo2\n"; }
    void bar() { std::cout << "bar2\n"; }


    inline static vtable_t holder{
            +[](ICoolBase* base) { static_cast<CoolDerived2*>(base)->foo(); },
            +[](ICoolBase* base) { static_cast<CoolDerived2*>(base)->bar(); }
    };
};

}


namespace Stuff
{

struct S {};

void swap(S& s1, S& s2)
{
    std::cout << "S's swap";
}

}

namespace Std
{

// Niebler
// Niebloid

inline namespace
{

inline constexpr struct TagInvoke {
    template<class CPO, class... Args>
    // requires
    void operator() (CPO cpo, Args... args) const
    {
        tag_invoke(cpo, (Args&&) args...);
    }
} tag_invoke;

}

}



namespace MyLib
{

inline constexpr struct Beg {
    template<class T>
    decltype(auto) operator() (T&& t) const {
        if constexpr (requires { t.begin(); })
        {
            return t.begin();
        }
        else
        {
            return tag_invoke(*this, (T&&) t);
        }
    }
} beg;

template<class T>
struct Adaptor
{
    template<class CPO>
    friend auto tag_invoke(CPO cpo, Adaptor<T> adaptor)
    {
        return Std::tag_invoke(cpo, adaptor.t);
    }


    T t;
};


}


template<class T, auto... CPOs>
concept trait = (requires(T t) { CPOs(t); } && ...);


//template<class T>
//void foo(T vec)
//{
//    MyLib::swap(vec[0], vec[1]);
//    std::ranges::begin(vec);
//}


template<class T>
concept range = trait<T, std::ranges::begin, std::ranges::end>;



int main()
{

//    foo(std::vector{Stuff::S{}, Stuff::S{}, Stuff::S{}});
//    foo(std::vector{0, 1, 2});

    MyLib::MyVector vec;

    auto it = MyLib::beg(vec);
    static_assert(std::same_as<decltype(it), std::vector<int>::iterator>);

    std::array<int, 3> a;
    auto it2 = MyLib::beg(a);

    return 0;
}

#include <atomic>
#include <utility>
#include <cassert>

template <typename T>
struct NonatomicPointerPolicy {
    explicit NonatomicPointerPolicy(T* ptr) : ptr_(ptr) {}
    T* GetPtr() {
        return ptr_;
    }
    T* ExchangePtr(T* ptr) {
        return std::exchange(ptr_, ptr);
    }

protected:
    T* ptr_;
};

template <typename T>
struct AtomicPointerPolicy {
    explicit AtomicPointerPolicy(T* ptr) : ptr_(ptr) {}
    T* GetPtr() {
        return ptr_.load();
    }
    T* ExchangePtr(T* ptr) {
        return ptr_.exchange(ptr);
    }

protected:
    std::atomic<T*> ptr_;
};

template <typename T>
struct UnsafeIndirectionPolicy {
    void CheckValidity(T* ptr) {}
};

template <typename T>
struct AssertIndirectionPolicy {
    operator UnsafeIndirectionPolicy<T>() { return {}; }
    void CheckValidity(T* ptr) { assert(ptr); }
};

template <typename T>
struct ExceptionIndirectionPolicy {
    void CheckValidity(T* ptr) { if (!ptr) { throw std::runtime_error("Null pointer exception!"); } }
};

template<class T,
         template <typename> class IndirectionPolicy = UnsafeIndirectionPolicy,
         template <typename> class MTPolicy = NonatomicPointerPolicy>
struct UniquePointer : IndirectionPolicy<T>, MTPolicy<T>
{
    UniquePointer() = default;
    explicit UniquePointer(T* ptr) : MTPolicy<T>(ptr) {}

    UniquePointer(UniquePointer&& other) noexcept {
        if (this != &other) { delete ExchangePtr(other.ExchangePtr(nullptr)); }
    }

    UniquePointer& operator=(UniquePointer&& other) noexcept {
        if (this != &other) { delete ExchangePtr(other.ExchangePtr(nullptr)); }
        return *this;
    }


    template<template <typename> class IndirectionPolicy2>
    explicit UniquePointer(UniquePointer<T, IndirectionPolicy2, MTPolicy>&& ptr) noexcept
        requires requires { IndirectionPolicy<T>(ptr); }
        : MTPolicy<T>(ptr.ExchangePtr(nullptr))
    {
        // if IndirectionPolicy2 == Unsafe
        // and IndirectionPolicy = Safe
        // prohibit cast
    }

    // operator bool?

    T& operator*() {
        T* myPtr = this->GetPtr();
        this->CheckValidity(myPtr);
        return *myPtr;
    }

    ~UniquePointer() noexcept { delete this->ExchangePtr(nullptr); }
};


int main()
{
    UniquePointer<int, AssertIndirectionPolicy> p1(new int(42));

    UniquePointer<int, UnsafeIndirectionPolicy> p2(std::move(p1));
}
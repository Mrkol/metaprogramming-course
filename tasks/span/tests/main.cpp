#include <testing/assert.hpp>
#include <Span.hpp>

#include <gtest/gtest.h>

#include <vector>
#include <numeric>
#include <algorithm>
#include <concepts>
#include <type_traits>



static_assert(sizeof(Span<int>) == sizeof(void*) + sizeof(std::size_t));

static_assert(sizeof(Span<int, 2>) == sizeof(void*));

EXPECT_STATIC_TRUE((requires(
      Span<int> s1,
      Span<int, 42> s2) {
    requires std::contiguous_iterator<decltype(s1.begin())>;
    requires std::contiguous_iterator<decltype(s1.end())>;
    requires std::contiguous_iterator<decltype(s2.begin())>;
    requires std::contiguous_iterator<decltype(s2.end())>;

    { s1.begin() } -> std::same_as<decltype(s1)::iterator>;
    { s1.end() } -> std::same_as<decltype(s1)::iterator>;

    { s2.begin() } -> std::same_as<decltype(s2)::iterator>;
    { s2.end() } -> std::same_as<decltype(s2)::iterator>;

    { s2.rbegin() } -> std::same_as<std::reverse_iterator<decltype(s2)::iterator>>;
    { s2.rend() } -> std::same_as<std::reverse_iterator<decltype(s2)::iterator>>;
  }))

EXPECT_STATIC_TRUE((requires() {
    requires std::same_as<Span<const int>::value_type, int>;
    requires std::same_as<Span<const int>::element_type, const int>;
    requires std::same_as<Span<const int>::size_type, std::size_t>;
    requires std::same_as<Span<int>::pointer, int*>;
    requires std::same_as<Span<int>::const_pointer, const int*>;
    requires std::same_as<Span<int>::reference, int&>;
    requires std::same_as<Span<int>::const_reference, const int&>;
    requires std::same_as<Span<const int>::difference_type, std::ptrdiff_t>;
  }))

EXPECT_STATIC_TRUE((requires(Span<const int> cSpan) {
    requires std::is_const_v<std::remove_reference_t<decltype(cSpan[0])>>;
  }));

EXPECT_STATIC_TRUE((requires() {
    requires std::copyable<Span<int>>;
    requires std::copyable<Span<int, 42>>;
    requires std::is_trivially_copyable_v<Span<int>>;
    requires std::is_trivially_copyable_v<Span<int, 42>>;
  }));

template<class T>
concept IsConstLvalRef = std::is_lvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>;

EXPECT_STATIC_TRUE((requires(Span<int const> s1, Span<int const, 42> s2, size_t idx) {
    { *s1.begin() } -> IsConstLvalRef;
    { *s1.end() } -> IsConstLvalRef;
    { *s2.begin() } -> IsConstLvalRef;
    { *s2.end() } -> IsConstLvalRef;
    { s1[idx] } -> IsConstLvalRef;
    { s2[idx] } -> IsConstLvalRef;
    { s1.Front() } -> IsConstLvalRef;
    { s2.Front() } -> IsConstLvalRef;
    { s1.Back() } -> IsConstLvalRef;
    { s2.Back() } -> IsConstLvalRef;
  }));

template<class T>
concept IsNonConstLvalRef = std::is_lvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>;

// Span variable being const simply means that you can't rebind it to a different container, not
// that you can't modify the data inside the underlying container through it!
EXPECT_STATIC_TRUE((requires(const Span<int> s1, const Span<int, 42> s2, size_t idx) {
    { *s1.begin() } -> IsNonConstLvalRef;
    { *s1.end() } -> IsNonConstLvalRef;
    { *s2.begin() } -> IsNonConstLvalRef;
    { *s2.end() } -> IsNonConstLvalRef;
    { s1[idx] } -> IsNonConstLvalRef;
    { s2[idx] } -> IsNonConstLvalRef;
    { s1.Front() } -> IsNonConstLvalRef;
    { s2.Front() } -> IsNonConstLvalRef;
    { s1.Back() } -> IsNonConstLvalRef;
    { s2.Back() } -> IsNonConstLvalRef;
  }));

TEST(SpanTests, SpanOverVector) {
  std::vector<int> vec(42);
  std::iota(vec.begin(), vec.end(), 0);

  Span all(vec);
  EXPECT_EQ(all.Size(), vec.size());
  for (auto elem : all)
    EXPECT_EQ(elem, vec[elem]);

  EXPECT_EQ(all.Front(), vec.front());
  EXPECT_EQ(all.Back(), vec.back());

  Span first10 = all.First(10);
  EXPECT_EQ(first10.Size(), 10u);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(first10[i], vec[i]);

  Span staticFirst10 = all.First<10>();
  static_assert(staticFirst10.Size() == 10);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(staticFirst10[i], vec[i]);

  Span last10 = all.Last(10);
  EXPECT_EQ(last10.Size(), 10u);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(last10[i], vec[42 - 10 + i]);

  Span staticLast10 = all.Last<10>();
  static_assert(staticLast10.Size() == 10);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(staticLast10[i], vec[42 - 10 + i]);
}

TEST(SpanTests, SpanOverArray) {
  std::array<int, 42> arr{};
  std::iota(arr.begin(), arr.end(), 0);

  Span all(arr);
  static_assert(all.Size() == arr.size());
  EXPECT_EQ(all.Data(), arr.data());
  for (auto elem : all)
    EXPECT_EQ(elem, arr[elem]);

  EXPECT_EQ(all.Front(), arr.front());
  EXPECT_EQ(all.Back(), arr.back());

  Span first10 = all.First(10);
  EXPECT_EQ(first10.Size(), 10u);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(first10[i], arr[i]);

  Span staticFirst10 = all.First<10>();
  static_assert(staticFirst10.Size() == 10);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(staticFirst10[i], arr[i]);

  Span last10 = all.Last(10);
  EXPECT_EQ(last10.Size(), 10u);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(last10[i], arr[42 - 10 + i]);

  Span staticLast10 = all.Last<10>();
  static_assert(staticLast10.Size() == 10);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(staticLast10[i], arr[42 - 10 + i]);
}

TEST(SpanTests, ConstSpan) {
  std::vector<int> vec(42);
  std::iota(vec.begin(), vec.end(), 0);
  const Span span(vec.begin(), vec.size());
  for (std::size_t i = 0; i < vec.size(); ++i)
    EXPECT_EQ(span[i], vec[i]);

  for (std::size_t i = 0; i < vec.size(); i += 2)
    ++span[i];

  for (std::size_t i = 0; i < span.Size(); ++i)
    EXPECT_EQ(span[i], vec[i]);

  Span<const int> cspan = vec;
  for (std::size_t i = 0; i < cspan.Size(); ++i)
    EXPECT_EQ(cspan[i], vec[i]);
}

TEST(SpanTests, SimpleBoundaryChecks) {
  std::vector<int> vec(10, 0);
  Span spanFull{vec};

  EXPECT_RUNTIME_FAIL(({
    Span<int, 100u> span{vec};
  }));

  EXPECT_RUNTIME_FAIL(({
    spanFull.First<11u>();
  }));

  EXPECT_RUNTIME_FAIL(({
    spanFull.First(11u);
  }));

  EXPECT_RUNTIME_FAIL(({
    spanFull.Last<11u>();
  }));

  EXPECT_RUNTIME_FAIL(({
    spanFull.Last(11u);
  }));

  Span<int> spanEmpty{(int*)nullptr, 0u};

  EXPECT_RUNTIME_FAIL(({
    spanEmpty.Front();
  }));

  EXPECT_RUNTIME_FAIL(({
    spanEmpty.Back();
  }));

  EXPECT_RUNTIME_FAIL(({
    spanEmpty[0];
  }));
}

TEST(SpanTests, Empty) {
  std::array<int, 0> arr = {};
  std::vector<int> vec = {};
  Span sp1{vec};
  Span<int, 0> sp2{arr};

  EXPECT_RUNTIME_FAIL(({
    sp1[0];
  }));

  EXPECT_RUNTIME_FAIL(({
    sp2[0];
  }));
}

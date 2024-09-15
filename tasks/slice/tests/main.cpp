#include <Slice.hpp>
#include <testing/assert.hpp>

#include <gtest/gtest.h>

#include <vector>
#include <numeric>
#include <algorithm>
#include <concepts>
#include <type_traits>



static_assert(sizeof(Slice<int>) == sizeof(void*) + sizeof(std::size_t));

static_assert(sizeof(Slice<int, 2>) == sizeof(void*));

static_assert(sizeof(Slice<int, 2, dynamic_stride>) == sizeof(void*) + sizeof(std::ptrdiff_t));

static_assert(sizeof(Slice<int, std::dynamic_extent, dynamic_stride>)
  == sizeof(void*) + sizeof(std::size_t) + sizeof(std::ptrdiff_t));

EXPECT_STATIC_TRUE((requires(
      Slice<int, 42, 42> s1,
      Slice<int, 42> s2) {
    requires std::random_access_iterator<decltype(s1.begin())>;
    requires std::random_access_iterator<decltype(s1.end())>;
    requires std::random_access_iterator<decltype(s2.begin())>;
    requires std::random_access_iterator<decltype(s2.end())>;

    { s1.begin() } -> std::same_as<decltype(s1)::iterator>;
    { s1.end() } -> std::same_as<decltype(s1)::iterator>;

    { s2.begin() } -> std::same_as<decltype(s2)::iterator>;
    { s2.end() } -> std::same_as<decltype(s2)::iterator>;

    { s2.rbegin() } -> std::same_as<std::reverse_iterator<decltype(s2)::iterator>>;
    { s2.rend() } -> std::same_as<std::reverse_iterator<decltype(s2)::iterator>>;
  }))

EXPECT_STATIC_TRUE((requires() {
    requires std::same_as<Slice<const int>::value_type, int>;
    requires std::same_as<Slice<const int>::element_type, const int>;
    requires std::same_as<Slice<const int>::size_type, std::size_t>;
    requires std::same_as<Slice<int>::pointer, int*>;
    requires std::same_as<Slice<int>::const_pointer, const int*>;
    requires std::same_as<Slice<int>::reference, int&>;
    requires std::same_as<Slice<int>::const_reference, const int&>;
    requires std::same_as<Slice<const int>::difference_type, std::ptrdiff_t>;
  }))

EXPECT_STATIC_TRUE((requires(Slice<const int> cslice) {
    requires std::is_const_v<std::remove_reference_t<decltype(cslice[0])>>;
  }));

EXPECT_STATIC_TRUE((requires() {
    requires std::copyable<Slice<int, 42, 42>>;
    requires std::equality_comparable<Slice<int, 42, 42>>;
    requires std::copyable<Slice<int, 42, dynamic_stride>>;
    requires std::equality_comparable<Slice<int, 42, dynamic_stride>>;
    requires std::regular<Slice<int, std::dynamic_extent, 42>>;
    requires std::regular<Slice<int, std::dynamic_extent, dynamic_stride>>;
    requires std::is_trivially_copyable_v<Slice<int, 42, 42>>;
    requires std::is_trivially_copyable_v<Slice<int, 42, dynamic_stride>>;
    requires std::is_trivially_copyable_v<Slice<int, std::dynamic_extent, 42>>;
    requires std::is_trivially_copyable_v<Slice<int, std::dynamic_extent, dynamic_stride>>;
  }));


EXPECT_STATIC_TRUE((requires() {
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, 42, 42>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, std::dynamic_extent, 42>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, 42, dynamic_stride>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, std::dynamic_extent, dynamic_stride>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<int, std::dynamic_extent, 42>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<int, 42, dynamic_stride>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<int, std::dynamic_extent, dynamic_stride>>;
  }));

TEST(SliceTests, SliceOverVector) {
  std::vector<int> vec(42);
  std::iota(vec.begin(), vec.end(), 0);

  Slice all(vec);
  EXPECT_EQ(all.Size(), vec.size());
  EXPECT_EQ(all.Stride(), 1u);
  for (auto elem : all)
    EXPECT_EQ(elem, vec[elem]);

  Slice even = all.Skip(2);
  EXPECT_EQ(even.Size(), vec.size() / 2);
  EXPECT_EQ(even.Stride(), 2u);
  for (auto elem : even)
    EXPECT_EQ(elem, 2*vec[elem/2]);

  Slice staticEven = all.Skip<2>();
  EXPECT_EQ(staticEven.Size(), vec.size() / 2);
  static_assert(staticEven.Stride() == 2);
  for (auto elem : staticEven)
    EXPECT_EQ(elem, 2*vec[elem/2]);

  Slice everyFourth1 = staticEven.Skip(2);
  EXPECT_EQ(everyFourth1.Size(), vec.size() / 4 + 1);
  EXPECT_EQ(everyFourth1.Stride(), 4u);
  for (auto elem : everyFourth1)
    EXPECT_EQ(elem, 4*vec[elem/4]);

  Slice everyFourth2 = even.Skip<2>();
  EXPECT_EQ(everyFourth2.Size(), vec.size() / 4 + 1);
  EXPECT_EQ(everyFourth2.Stride(), 4u);
  for (auto elem : everyFourth2)
    EXPECT_EQ(elem, 4*vec[elem/4]);

  Slice everyFourth3 = staticEven.Skip<2>();
  EXPECT_EQ(everyFourth3.Size(), vec.size() / 4 + 1);
  static_assert(everyFourth3.Stride() == 4);
  for (auto elem : everyFourth3)
    EXPECT_EQ(elem, 4*vec[elem/4]);

  Slice first10 = all.First(10);
  EXPECT_EQ(first10.Size(), 10u);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(first10[i], vec[i]);

  Slice staticFirst10 = all.First<10>();
  static_assert(staticFirst10.Size() == 10);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(staticFirst10[i], vec[i]);

  Slice last10 = all.Last(10);
  EXPECT_EQ(last10.Size(), 10u);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(last10[i], vec[42 - 10 + i]);

  Slice staticLast10 = all.Last<10>();
  static_assert(staticLast10.Size() == 10);
  for (std::size_t i = 0; i < 10; ++i)
    EXPECT_EQ(staticLast10[i], vec[42 - 10 + i]);

  EXPECT_EQ(first10, all.DropLast(42 - 10));
  EXPECT_EQ(first10, all.DropLast<42 - 10>());
  EXPECT_EQ(staticFirst10, all.DropLast<42 - 10>());
  EXPECT_EQ(staticFirst10, all.DropLast(42 - 10));

  EXPECT_EQ(last10, all.DropFirst(42 - 10));
  EXPECT_EQ(last10, all.DropFirst<42 - 10>());
  EXPECT_EQ(staticLast10, all.DropFirst<42 - 10>());
  EXPECT_EQ(staticLast10, all.DropFirst(42 - 10));
}

TEST(SliceTests, SliceOverArray) {
  std::array<int, 42> arr{};
  std::iota(arr.begin(), arr.end(), 0);


  Slice all(arr);
  static_assert(all.Size() == arr.size());
  EXPECT_EQ(all.Stride(), 1u);
  for (auto elem : all)
    EXPECT_EQ(elem, arr[elem]);

  Slice even = all.Skip(2);
  EXPECT_EQ(even.Size(), arr.size() / 2);
  EXPECT_EQ(even.Stride(), 2u);
  for (auto elem : even)
    EXPECT_EQ(elem, 2*arr[elem/2]);

  Slice staticEven = all.Skip<2>();
  static_assert(staticEven.Size() == arr.size() / 2);
  static_assert(staticEven.Stride() == 2);
  for (auto elem : staticEven)
    EXPECT_EQ(elem, 2*arr[elem/2]);

  Slice everyFourth1 = staticEven.Skip(2);
  EXPECT_EQ(everyFourth1.Size(), arr.size() / 4 + 1);
  EXPECT_EQ(everyFourth1.Stride(), 4u);
  for (auto elem : everyFourth1)
    EXPECT_EQ(elem, 4*arr[elem/4]);

  Slice everyFourth2 = even.Skip<2>();
  EXPECT_EQ(everyFourth2.Size(), arr.size() / 4 + 1);
  EXPECT_EQ(everyFourth2.Stride(), 4u);
  for (auto elem : everyFourth2)
    EXPECT_EQ(elem, 4*arr[elem/4]);

  Slice everyFourth3 = staticEven.Skip<2>();
  static_assert(everyFourth3.Size() == arr.size() / 4 + 1);
  static_assert(everyFourth3.Stride() == 4);
  for (auto elem : everyFourth3)
    EXPECT_EQ(elem, 4*arr[elem/4]);

  // Implicit casts to dynamic parameters
  Slice<int, everyFourth3.Size(), dynamic_stride>
    dynStrideSlice = everyFourth3;
  EXPECT_EQ(dynStrideSlice.Data(), everyFourth3.Data());
  EXPECT_EQ(dynStrideSlice.Size(), everyFourth3.Size());
  EXPECT_EQ(dynStrideSlice.Stride(), everyFourth3.Stride());

  Slice<int, std::dynamic_extent, everyFourth3.Stride()>
    dynExtentSlice = everyFourth3;
  EXPECT_EQ(dynExtentSlice.Data(), everyFourth3.Data());
  EXPECT_EQ(dynExtentSlice.Size(), everyFourth3.Size());
  EXPECT_EQ(dynExtentSlice.Stride(), everyFourth3.Stride());

  Slice<int, std::dynamic_extent, dynamic_stride>
    dynSlice = everyFourth3;
  EXPECT_EQ(dynSlice.Data(), everyFourth3.Data());
  EXPECT_EQ(dynSlice.Size(), everyFourth3.Size());
  EXPECT_EQ(dynSlice.Stride(), everyFourth3.Stride());
}

TEST(SliceTests, ConstSlice) {
  std::vector<int> vec(42);
  std::iota(vec.begin(), vec.end(), 0);
  const Slice slice(vec.begin(), vec.size() / 3, 3);
  EXPECT_EQ(vec.size() / 3, slice.Size());
  for (std::size_t i = 0; i < vec.size(); i += 3)
    EXPECT_EQ(slice[i/3], vec[i]);

  for (std::size_t i = 0; i < vec.size(); i += 3)
    ++slice[i/3];

  Slice<const int> cslice = vec;
  for (std::size_t i = 0; i < cslice.Size(); ++i)
    EXPECT_EQ(cslice[i], vec[i]);
}

TEST(SliceTests, EqualityComparison) {
  std::array<int, 42> arr;
  std::iota(arr.begin(), arr.end(), 0);
  Slice all = arr;

  Slice<const int> constAll = all;
  EXPECT_EQ(all, constAll);

  Slice<const int, std::dynamic_extent, 1> constDynExtAll = all;
  EXPECT_EQ(all, constDynExtAll);

  Slice<const int, all.Size(), dynamic_stride> constDynStrideAll = all;
  EXPECT_EQ(all, constDynStrideAll);

  Slice<const int, std::dynamic_extent, dynamic_stride> constDynStrideDynextAll = all;
  EXPECT_EQ(all, constDynStrideDynextAll);

  Slice<int, std::dynamic_extent, 1> dynExtAll = all;
  EXPECT_EQ(all, dynExtAll);

  Slice<int, all.Size(), dynamic_stride> dynStrideAll = all;
  EXPECT_EQ(all, dynStrideAll);

  Slice<int, std::dynamic_extent, dynamic_stride> dynStrideDynextAll = all;
  EXPECT_EQ(all, dynStrideDynextAll);
}

TEST(SliceTests, TrickyStrides) {
  std::array<int, 42> arr;
  std::iota(arr.begin(), arr.end(), 0);
  Slice all = arr;

  Slice<int, 21, 2> even1(arr.begin(), arr.size() / 2, 2);
  Slice even2 = all.Skip<2>();
  Slice even3 = all.Skip(2);
  EXPECT_EQ(even1, even2);
  EXPECT_EQ(even1, even3);

  Slice<int, 5, 10> tenth1(arr.data(), 5, 10);
  Slice tenth2 = all.Skip(10);
  Slice tenth3 = all.Skip<10>();
  EXPECT_EQ(tenth1, tenth2);
  EXPECT_EQ(tenth1, tenth3);

  Slice<int, 3, 10> someLast1(arr.data() + 20, 3, 10);
  Slice someLast2 = tenth1.DropFirst(2);
  Slice someLast3 = tenth1.Last(3);
  Slice someLast4 = tenth1.DropFirst<2>();
  Slice someLast5 = tenth1.Last<3>();
  EXPECT_EQ(someLast1, someLast2);
  EXPECT_EQ(someLast2, someLast3);
  EXPECT_EQ(someLast3, someLast4);
  EXPECT_EQ(someLast4, someLast5);
  EXPECT_TRUE(std::ranges::equal(someLast5, std::array{20, 30, 40}));
  EXPECT_EQ(someLast1[1], 30);
  EXPECT_EQ(someLast2[1], 30);
  EXPECT_EQ(someLast3[1], 30);
  EXPECT_EQ(someLast4[1], 30);
  EXPECT_EQ(someLast5[1], 30);

  std::vector<int> arr2(arr.begin(), arr.end());
  Slice<int, 3, 10> someFirst1(arr2.data(), 3, 10);
  Slice someFirst2 = Slice(arr2).Skip<2>().Skip(5).DropLast<2>();
  Slice someFirst3 = Slice(arr).Skip<5>().Skip<2>().First(3);
  EXPECT_TRUE(std::ranges::equal(someFirst1, someFirst2));
  EXPECT_TRUE(std::ranges::equal(someFirst2, someFirst3));
  EXPECT_TRUE(std::ranges::equal(someFirst1, someFirst3));
  EXPECT_TRUE(std::ranges::equal(someFirst1, std::array{0, 10, 20}));
  EXPECT_EQ(someFirst1[2], 20);
  EXPECT_EQ(someFirst2[2], 20);
  EXPECT_EQ(someFirst3[2], 20);
  EXPECT_EQ(someFirst2.Size(), 3u);
  EXPECT_EQ(someFirst3.Size(), 3u);
  static_assert(someFirst1.Size() == 3);
  static_assert(someFirst3.Stride() == 10);
}

TEST(SliceTests, RuntimeChecks) {
  std::vector<int> vec(10, 0);
  Slice<int> sliceStride1{vec};
  Slice<int, 3u, 3u> sliceStride3{vec};
  Slice<int> emptySlice{(int*)nullptr, 0u};

  EXPECT_RUNTIME_FAIL(({
    Slice<int, 100u> slice{vec};
  }));

  EXPECT_RUNTIME_OK(({
    Slice<int, 10u, 1> slice{vec};
  }));

  EXPECT_RUNTIME_FAIL(({
    Slice<int, std::dynamic_extent, 5u> slice{vec, /*stride*/ 1};
  }));

  EXPECT_RUNTIME_OK(({
    Slice<int, std::dynamic_extent, 11u> slice{vec};
  }));

  EXPECT_RUNTIME_FAIL(({
    emptySlice[0];
  }));

  EXPECT_RUNTIME_FAIL(({
    emptySlice.DropFirst(1);
  }));

  EXPECT_RUNTIME_FAIL(({
    emptySlice.DropFirst<1>();
  }));

  EXPECT_RUNTIME_FAIL(({
    emptySlice.DropLast(1);
  }));

  EXPECT_RUNTIME_FAIL(({
    emptySlice.DropLast<1>();
  }));

  EXPECT_RUNTIME_FAIL(({
    sliceStride1[11];
  }));

  EXPECT_RUNTIME_FAIL(({
    sliceStride3[4];
  }));

  EXPECT_RUNTIME_OK(({
    sliceStride3.Skip(1000);
  }));

  EXPECT_RUNTIME_OK(({
    sliceStride3.Skip<1000>();
  }));

  EXPECT_RUNTIME_FAIL(({
    for (auto iter = std::begin(sliceStride3); ; ++iter) {
    }
  }));

  EXPECT_RUNTIME_FAIL(({
    for (auto iter = std::end(sliceStride3); ; --iter) {
    }
  }));
}

TEST(SliceTests, EmptySlice) {
  std::array<int, 0> arr = {};
  std::vector<int> vec = {};
  Slice sl1{vec};
  Slice<int, 0, 1u> sl2{arr};

  EXPECT_RUNTIME_FAIL(({
    sl1[0];
  }));

  EXPECT_RUNTIME_FAIL(({
    sl2[0];
  }));
}

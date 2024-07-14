#include <Slice.hpp>
#include <commons/assert.hpp>

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

MPC_STATIC_REQUIRE_TRUE((requires(
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

MPC_STATIC_REQUIRE_TRUE((requires() {
    requires std::same_as<Slice<const int>::value_type, int>;
    requires std::same_as<Slice<const int>::element_type, const int>;
    requires std::same_as<Slice<const int>::size_type, std::size_t>;
    requires std::same_as<Slice<int>::pointer, int*>;
    requires std::same_as<Slice<int>::const_pointer, const int*>;
    requires std::same_as<Slice<int>::reference, int&>;
    requires std::same_as<Slice<int>::const_reference, const int&>;
    requires std::same_as<Slice<const int>::difference_type, std::ptrdiff_t>;
  }))

MPC_STATIC_REQUIRE_TRUE((requires(Slice<const int> cslice) {
    requires std::is_const_v<std::remove_reference_t<decltype(cslice[0])>>;
  }));

MPC_STATIC_REQUIRE_TRUE((requires() {
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


MPC_STATIC_REQUIRE_TRUE((requires() {
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, 42, 42>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, std::dynamic_extent, 42>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, 42, dynamic_stride>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, std::dynamic_extent, dynamic_stride>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<int, std::dynamic_extent, 42>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<int, 42, dynamic_stride>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<int, std::dynamic_extent, dynamic_stride>>;
  }));

int main(int, char**)
{
  {
    std::vector<int> vec(42);
    std::iota(vec.begin(), vec.end(), 0);

    Slice all(vec);
    MPC_REQUIRE(eq, all.Size(), vec.size());
    MPC_REQUIRE(eq, all.Stride(), 1u);
    for (auto elem : all)
      MPC_REQUIRE(eq, elem, vec[elem]);

    Slice even = all.Skip(2);
    MPC_REQUIRE(eq, even.Size(), vec.size() / 2);
    MPC_REQUIRE(eq, even.Stride(), 2u);
    for (auto elem : even)
      MPC_REQUIRE(eq, elem, 2*vec[elem/2]);

    Slice staticEven = all.Skip<2>();
    MPC_REQUIRE(eq, staticEven.Size(), vec.size() / 2);
    static_assert(staticEven.Stride() == 2);
    for (auto elem : staticEven)
      MPC_REQUIRE(eq, elem, 2*vec[elem/2]);

    Slice everyFourth1 = staticEven.Skip(2);
    MPC_REQUIRE(eq, everyFourth1.Size(), vec.size() / 4 + 1);
    MPC_REQUIRE(eq, everyFourth1.Stride(), 4u);
    for (auto elem : everyFourth1)
      MPC_REQUIRE(eq, elem, 4*vec[elem/4]);

    Slice everyFourth2 = even.Skip<2>();
    MPC_REQUIRE(eq, everyFourth2.Size(), vec.size() / 4 + 1);
    MPC_REQUIRE(eq, everyFourth2.Stride(), 4u);
    for (auto elem : everyFourth2)
      MPC_REQUIRE(eq, elem, 4*vec[elem/4]);

    Slice everyFourth3 = staticEven.Skip<2>();
    MPC_REQUIRE(eq, everyFourth3.Size(), vec.size() / 4 + 1);
    static_assert(everyFourth3.Stride() == 4);
    for (auto elem : everyFourth3)
      MPC_REQUIRE(eq, elem, 4*vec[elem/4]);

    Slice first10 = all.First(10);
    MPC_REQUIRE(eq, first10.Size(), 10u);
    for (std::size_t i = 0; i < 10; ++i)
      MPC_REQUIRE(eq, first10[i], vec[i]);

    Slice staticFirst10 = all.First<10>();
    static_assert(staticFirst10.Size() == 10);
    for (std::size_t i = 0; i < 10; ++i)
      MPC_REQUIRE(eq, staticFirst10[i], vec[i]);

    Slice last10 = all.Last(10);
    MPC_REQUIRE(eq, last10.Size(), 10u);
    for (std::size_t i = 0; i < 10; ++i)
      MPC_REQUIRE(eq, last10[i], vec[42 - 10 + i]);

    Slice staticLast10 = all.Last<10>();
    static_assert(staticLast10.Size() == 10);
    for (std::size_t i = 0; i < 10; ++i)
      MPC_REQUIRE(eq, staticLast10[i], vec[42 - 10 + i]);

    MPC_REQUIRE(eq, first10, all.DropLast(42 - 10));
    MPC_REQUIRE(eq, first10, all.DropLast<42 - 10>());
    MPC_REQUIRE(eq, staticFirst10, all.DropLast<42 - 10>());
    MPC_REQUIRE(eq, staticFirst10, all.DropLast(42 - 10));

    MPC_REQUIRE(eq, last10, all.DropFirst(42 - 10));
    MPC_REQUIRE(eq, last10, all.DropFirst<42 - 10>());
    MPC_REQUIRE(eq, staticLast10, all.DropFirst<42 - 10>());
    MPC_REQUIRE(eq, staticLast10, all.DropFirst(42 - 10));
  }


  {
    std::array<int, 42> arr{};
    std::iota(arr.begin(), arr.end(), 0);


    Slice all(arr);
    static_assert(all.Size() == arr.size());
    MPC_REQUIRE(eq, all.Stride(), 1u);
    for (auto elem : all)
      MPC_REQUIRE(eq, elem, arr[elem]);

    Slice even = all.Skip(2);
    MPC_REQUIRE(eq, even.Size(), arr.size() / 2);
    MPC_REQUIRE(eq, even.Stride(), 2u);
    for (auto elem : even)
      MPC_REQUIRE(eq, elem, 2*arr[elem/2]);

    Slice staticEven = all.Skip<2>();
    static_assert(staticEven.Size() == arr.size() / 2);
    static_assert(staticEven.Stride() == 2);
    for (auto elem : staticEven)
      MPC_REQUIRE(eq, elem, 2*arr[elem/2]);

    Slice everyFourth1 = staticEven.Skip(2);
    MPC_REQUIRE(eq, everyFourth1.Size(), arr.size() / 4 + 1);
    MPC_REQUIRE(eq, everyFourth1.Stride(), 4u);
    for (auto elem : everyFourth1)
      MPC_REQUIRE(eq, elem, 4*arr[elem/4]);

    Slice everyFourth2 = even.Skip<2>();
    MPC_REQUIRE(eq, everyFourth2.Size(), arr.size() / 4 + 1);
    MPC_REQUIRE(eq, everyFourth2.Stride(), 4u);
    for (auto elem : everyFourth2)
      MPC_REQUIRE(eq, elem, 4*arr[elem/4]);

    Slice everyFourth3 = staticEven.Skip<2>();
    static_assert(everyFourth3.Size() == arr.size() / 4 + 1);
    static_assert(everyFourth3.Stride() == 4);
    for (auto elem : everyFourth3)
      MPC_REQUIRE(eq, elem, 4*arr[elem/4]);

    // Implicit casts to dynamic parameters
    Slice<int, everyFourth3.Size(), dynamic_stride>
      dynStrideSlice = everyFourth3;
    MPC_REQUIRE(eq, dynStrideSlice.Data(), everyFourth3.Data());
    MPC_REQUIRE(eq, dynStrideSlice.Size(), everyFourth3.Size());
    MPC_REQUIRE(eq, dynStrideSlice.Stride(), everyFourth3.Stride());

    Slice<int, std::dynamic_extent, everyFourth3.Stride()>
      dynExtentSlice = everyFourth3;
    MPC_REQUIRE(eq, dynExtentSlice.Data(), everyFourth3.Data());
    MPC_REQUIRE(eq, dynExtentSlice.Size(), everyFourth3.Size());
    MPC_REQUIRE(eq, dynExtentSlice.Stride(), everyFourth3.Stride());

    Slice<int, std::dynamic_extent, dynamic_stride>
      dynSlice = everyFourth3;
    MPC_REQUIRE(eq, dynSlice.Data(), everyFourth3.Data());
    MPC_REQUIRE(eq, dynSlice.Size(), everyFourth3.Size());
    MPC_REQUIRE(eq, dynSlice.Stride(), everyFourth3.Stride());
  }

  {
    std::vector<int> vec(42);
    std::iota(vec.begin(), vec.end(), 0);
    const Slice slice(vec.begin(), vec.size() / 3, 3);
    MPC_REQUIRE(eq, vec.size() / 3, slice.Size());
    for (std::size_t i = 0; i < vec.size(); i += 3)
      MPC_REQUIRE(eq, slice[i/3], vec[i]);

    for (std::size_t i = 0; i < vec.size(); i += 3)
      ++slice[i/3];

    Slice<const int> cslice = vec;
    for (std::size_t i = 0; i < cslice.Size(); ++i)
      MPC_REQUIRE(eq, cslice[i], vec[i]);
  }

  {
    std::array<int, 42> arr;
    std::iota(arr.begin(), arr.end(), 0);
    Slice all = arr;

    Slice<const int> constAll = all;
    MPC_REQUIRE(eq, all, constAll);

    Slice<const int, std::dynamic_extent, 1> constDynExtAll = all;
    MPC_REQUIRE(eq, all, constDynExtAll);

    Slice<const int, all.Size(), dynamic_stride> constDynStrideAll = all;
    MPC_REQUIRE(eq, all, constDynStrideAll);

    Slice<const int, std::dynamic_extent, dynamic_stride> constDynStrideDynextAll = all;
    MPC_REQUIRE(eq, all, constDynStrideDynextAll);

    Slice<int, std::dynamic_extent, 1> dynExtAll = all;
    MPC_REQUIRE(eq, all, dynExtAll);

    Slice<int, all.Size(), dynamic_stride> dynStrideAll = all;
    MPC_REQUIRE(eq, all, dynStrideAll);

    Slice<int, std::dynamic_extent, dynamic_stride> dynStrideDynextAll = all;
    MPC_REQUIRE(eq, all, dynStrideDynextAll);
  }

  {
    std::array<int, 42> arr;
    std::iota(arr.begin(), arr.end(), 0);
    Slice all = arr;

    Slice<int, 21, 2> even1(arr.begin(), arr.size() / 2, 2);
    Slice even2 = all.Skip<2>();
    Slice even3 = all.Skip(2);
    MPC_REQUIRE(eq, even1, even2);
    MPC_REQUIRE(eq, even1, even3);

    Slice<int, 5, 10> tenth1(arr.data(), 5, 10);
    Slice tenth2 = all.Skip(10);
    Slice tenth3 = all.Skip<10>();
    MPC_REQUIRE(eq, tenth1, tenth2);
    MPC_REQUIRE(eq, tenth1, tenth3);

    Slice<int, 3, 10> someLast1(arr.data() + 20, 3, 10);
    Slice someLast2 = tenth1.DropFirst(2);
    Slice someLast3 = tenth1.Last(3);
    Slice someLast4 = tenth1.DropFirst<2>();
    Slice someLast5 = tenth1.Last<3>();
    MPC_REQUIRE(eq, someLast1, someLast2);
    MPC_REQUIRE(eq, someLast2, someLast3);
    MPC_REQUIRE(eq, someLast3, someLast4);
    MPC_REQUIRE(eq, someLast4, someLast5);
    MPC_REQUIRE(true, std::ranges::equal(someLast5, std::array{20, 30, 40}));
    MPC_REQUIRE(eq, someLast1[1], 30);
    MPC_REQUIRE(eq, someLast2[1], 30);
    MPC_REQUIRE(eq, someLast3[1], 30);
    MPC_REQUIRE(eq, someLast4[1], 30);
    MPC_REQUIRE(eq, someLast5[1], 30);

    std::vector<int> arr2(arr.begin(), arr.end());
    Slice<int, 3, 10> someFirst1(arr2.data(), 3, 10);
    Slice someFirst2 = Slice(arr2).Skip<2>().Skip(5).DropLast<2>();
    Slice someFirst3 = Slice(arr).Skip<5>().Skip<2>().First(3);
    MPC_REQUIRE(true, std::ranges::equal(someFirst1, someFirst2));
    MPC_REQUIRE(true, std::ranges::equal(someFirst2, someFirst3));
    MPC_REQUIRE(true, std::ranges::equal(someFirst1, someFirst3));
    MPC_REQUIRE(true, std::ranges::equal(someFirst1, std::array{0, 10, 20}));
    MPC_REQUIRE(eq, someFirst1[2], 20);
    MPC_REQUIRE(eq, someFirst2[2], 20);
    MPC_REQUIRE(eq, someFirst3[2], 20);
    MPC_REQUIRE(eq, someFirst2.Size(), 3u);
    MPC_REQUIRE(eq, someFirst3.Size(), 3u);
    static_assert(someFirst1.Size() == 3);
    static_assert(someFirst3.Stride() == 10);
  }
}

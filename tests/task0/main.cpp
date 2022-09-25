#include <Slice.hpp>
#include <commons/assert.hpp>

#include <vector>
#include <numeric>
#include <algorithm>
#include <concepts>
#include <type_traits>



static_assert(sizeof(Slice<int>) == sizeof(void*) + sizeof(size_t));

static_assert(sizeof(Slice<int, 2>) == sizeof(void*));

static_assert(sizeof(Slice<int, 2, std::dynamic_extent>) == sizeof(void*) + sizeof(size_t));

static_assert(sizeof(Slice<int, std::dynamic_extent, std::dynamic_extent>)
  == sizeof(void*) + 2*sizeof(size_t));

MPC_STATIC_REQUIRE_TRUE((requires(
      Slice<int, 42, 42> s1,
      Slice<int, 42> s2) {
    requires std::random_access_iterator<decltype(s1.begin())>;
    requires std::random_access_iterator<decltype(s1.end())>;
    requires std::random_access_iterator<decltype(s2.begin())>;
    requires std::random_access_iterator<decltype(s2.end())>;
    { s1.begin() } -> std::same_as<decltype(s1)::Iterator>;
    { s1.end() } -> std::same_as<decltype(s1)::Iterator>;
    { s2.begin() } -> std::same_as<decltype(s2)::Iterator>;
    { s2.end() } -> std::same_as<decltype(s2)::Iterator>;
  }))

MPC_STATIC_REQUIRE_TRUE((requires(Slice<const int> cslice) {
    requires std::is_const_v<std::remove_reference_t<decltype(cslice[0])>>;
  }));

MPC_STATIC_REQUIRE_TRUE((requires() {
    requires std::regular<Slice<int, 42, 42>>;
    requires std::regular<Slice<int, 42, std::dynamic_extent>>;
    requires std::regular<Slice<int, std::dynamic_extent, 42>>;
    requires std::regular<Slice<int, std::dynamic_extent, std::dynamic_extent>>;
  }));
  

MPC_STATIC_REQUIRE_TRUE((requires() {
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, 42, 42>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, std::dynamic_extent, 42>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, 42, std::dynamic_extent>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<const int, std::dynamic_extent, std::dynamic_extent>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<int, std::dynamic_extent, 42>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<int, 42, std::dynamic_extent>>;
    requires std::convertible_to<Slice<int, 42, 42>, Slice<int, std::dynamic_extent, std::dynamic_extent>>;
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
    MPC_REQUIRE(eq, everyFourth1.Size(), vec.size() / 4);
    MPC_REQUIRE(eq, everyFourth1.Stride(), 4u);
    for (auto elem : everyFourth1)
      MPC_REQUIRE(eq, elem, 4*vec[elem/4]);

    Slice everyFourth2 = even.Skip<2>();
    MPC_REQUIRE(eq, everyFourth2.Size(), vec.size() / 4);
    MPC_REQUIRE(eq, everyFourth2.Stride(), 4u);
    for (auto elem : everyFourth2)
      MPC_REQUIRE(eq, elem, 4*vec[elem/4]);

    Slice everyFourth3 = staticEven.Skip<2>();
    MPC_REQUIRE(eq, everyFourth3.Size(), vec.size() / 4);
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
    MPC_REQUIRE(eq, everyFourth1.Size(), arr.size() / 4);
    MPC_REQUIRE(eq, everyFourth1.Stride(), 4u);
    for (auto elem : everyFourth1)
      MPC_REQUIRE(eq, elem, 4*arr[elem/4]);

    Slice everyFourth2 = even.Skip<2>();
    MPC_REQUIRE(eq, everyFourth2.Size(), arr.size() / 4);
    MPC_REQUIRE(eq, everyFourth2.Stride(), 4u);
    for (auto elem : everyFourth2)
      MPC_REQUIRE(eq, elem, 4*arr[elem/4]);

    Slice everyFourth3 = staticEven.Skip<2>();
    static_assert(everyFourth3.Size() == arr.size() / 4);
    static_assert(everyFourth3.Stride() == 4);
    for (auto elem : everyFourth3)
      MPC_REQUIRE(eq, elem, 4*arr[elem/4]);

    // Implicit casts to dynamic parameters
    Slice<int, everyFourth3.Size(), std::dynamic_extent>
      dynStrideSlice = everyFourth3;
    MPC_REQUIRE(eq, dynStrideSlice.Data(), everyFourth3.Data());
    MPC_REQUIRE(eq, dynStrideSlice.Size(), everyFourth3.Size());
    MPC_REQUIRE(eq, dynStrideSlice.Stride(), everyFourth3.Stride());

    Slice<int, std::dynamic_extent, everyFourth3.Stride()>
      dynExtentSlice = everyFourth3;
    MPC_REQUIRE(eq, dynExtentSlice.Data(), everyFourth3.Data());
    MPC_REQUIRE(eq, dynExtentSlice.Size(), everyFourth3.Size());
    MPC_REQUIRE(eq, dynExtentSlice.Stride(), everyFourth3.Stride());

    Slice<int, std::dynamic_extent, std::dynamic_extent>
      dynSlice = everyFourth3;
    MPC_REQUIRE(eq, dynSlice.Data(), everyFourth3.Data());
    MPC_REQUIRE(eq, dynSlice.Size(), everyFourth3.Size());
    MPC_REQUIRE(eq, dynSlice.Stride(), everyFourth3.Stride());
  }

  {
    std::vector<int> vec(42);
    std::iota(vec.begin(), vec.end(), 0);
    const Slice slice(vec.begin(), vec.size(), 3);
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
    
    Slice<const int, all.Size(), std::dynamic_extent> constDynStrideAll = all;
    MPC_REQUIRE(eq, all, constDynStrideAll);
    
    Slice<const int, std::dynamic_extent, std::dynamic_extent> constDynStrideDynextAll = all;
    MPC_REQUIRE(eq, all, constDynStrideDynextAll);
    
    Slice<int, std::dynamic_extent, 1> dynExtAll = all;
    MPC_REQUIRE(eq, all, dynExtAll);
    
    Slice<int, all.Size(), std::dynamic_extent> dynStrideAll = all;
    MPC_REQUIRE(eq, all, dynStrideAll);
    
    Slice<int, std::dynamic_extent, std::dynamic_extent> dynStrideDynextAll = all;
    MPC_REQUIRE(eq, all, dynStrideDynextAll);
  }
}

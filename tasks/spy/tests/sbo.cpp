#include <testing/RegularityWitness.hpp>
#include <testing/assert.hpp>

#include "mocks.hpp"

#include <Spy.hpp>


template<class T, bool reallocate>
class SpyAllocator : private std::allocator<T> {
public:
  SpyAllocator() = default;
  SpyAllocator(std::size_t id) : id_{id} {}

  friend bool operator==(const SpyAllocator&, const SpyAllocator&) = default;

  using propagate_on_container_copy_assignment = std::integral_constant<bool, !reallocate>;
  using propagate_on_container_move_assignment = std::integral_constant<bool, !reallocate>;

  using value_type = T;

  template<class U>
  struct rebind { using other = SpyAllocator<U, reallocate>; };

  [[nodiscard]] constexpr T* allocate(std::size_t n) {
    ++allocationCounter_;
    return std::allocator_traits<std::allocator<T>>::allocate(*this, n);
  }

  constexpr void deallocate(T* p, std::size_t n) {
    return std::allocator_traits<std::allocator<T>>::deallocate(*this, p, n);
  }

  template<class U, class... Args>
  constexpr void construct(U* p, Args&&... args) {
    ++placementCounter_;
    std::allocator_traits<std::allocator<T>>::template construct<U>(*this, p, std::forward<Args>(args)...);
  }

  template<class U>
  constexpr void destroy(U* p) {
    std::allocator_traits<std::allocator<T>>::template destroy<U>(*this, p);
  }

  static void resetCounters() {
    allocationCounter_ = 0;
    placementCounter_ = 0;
  }

  static std::size_t allocationCount() { return allocationCounter_; }
  static std::size_t placementCount() { return placementCounter_; }

private:
  std::size_t id_ = 0;
  inline static std::size_t allocationCounter_ = 0;
  inline static std::size_t placementCounter_ = 0;
};

// Standard version of this is C++23
constexpr std::size_t operator "" _z ( unsigned long long n ) { return static_cast<std::size_t>(n); }

template<std::size_t padding, bool reallocate>
void runTest() {
  using mpc::detail::LoggerChecker;
  using mpc::detail::Counter;
  using mpc::detail::ValueLog;

  constexpr auto semiregular_opt = mpc::RegularityPolicy{};

  using Value = Counter<semiregular_opt>;
  using Alloc = SpyAllocator<std::byte, reallocate>;
  Alloc::resetCounters();

  auto s = Spy<Value, Alloc>{};
  MPC_REQUIRE(eq, Alloc::allocationCount(), 0_z);
  MPC_REQUIRE(eq, Alloc::placementCount(), 0_z);

  // Arbitrary decently-sized constant, don't use it as your SBO size please.
  constexpr bool EXPECT_SBO = padding <= 256;

  const std::size_t perCopyAllocInc = !EXPECT_SBO ? 1 : 0;
  const std::size_t perMoveAssignAllocInc = !EXPECT_SBO && reallocate ? 1 : 0;
  const std::size_t perMoveAssignPlaceInc = EXPECT_SBO || reallocate ? 1 : 0;
  const std::size_t perMoveConstructPlaceInc = EXPECT_SBO ? 1 : 0;

  LoggerChecker<semiregular_opt, padding> checker;
  s.setLogger(checker.getLogger());
  MPC_REQUIRE(eq, Alloc::allocationCount(), perCopyAllocInc);
  MPC_REQUIRE(eq, Alloc::placementCount(), 1_z);
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{});

  s->x = 0;
  MPC_REQUIRE(eq, checker.pollValues(), ValueLog{1}); 

  s.setLogger(checker.getLogger());
  MPC_REQUIRE(eq, Alloc::allocationCount(), 2*perCopyAllocInc);
  MPC_REQUIRE(eq, Alloc::placementCount(), 2_z);

  // Copy construct
  auto s1 = s;
  MPC_REQUIRE(eq, Alloc::allocationCount(), 3*perCopyAllocInc);
  MPC_REQUIRE(eq, Alloc::placementCount(), 3_z);

  // Copy assign
  Spy<Value, Alloc> s2;
  s2 = s;
  MPC_REQUIRE(eq, Alloc::allocationCount(), 4*perCopyAllocInc);
  MPC_REQUIRE(eq, Alloc::placementCount(), 4_z);

  // Move assign. Unequal alloc, expect reallocations
  Spy<Value, Alloc> ss(Alloc{1});
  ss = std::move(s);
  MPC_REQUIRE(eq, Alloc::allocationCount(), 4*perCopyAllocInc + perMoveAssignAllocInc);
  MPC_REQUIRE(eq, Alloc::placementCount(), 4_z + perMoveAssignPlaceInc);

  // Move construct, always grab allocator
  auto sss = std::move(ss);
  MPC_REQUIRE(eq, Alloc::allocationCount(), 4*perCopyAllocInc + perMoveAssignAllocInc);
  MPC_REQUIRE(eq, Alloc::placementCount(), 4_z + perMoveAssignPlaceInc + perMoveConstructPlaceInc);
}

TEST(SpyTest, BunchOfAllocTests)
{
  runTest<0, false>();
  runTest<0, true>();
  runTest<4096, false>();
  runTest<4096, true>();
}

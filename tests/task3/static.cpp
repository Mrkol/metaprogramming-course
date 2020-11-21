#include <spy.hpp>
#include "managed_ops.hpp"
#include <concepts>


namespace mpg::detail {

template <class Logger, class T>
concept ProperSpyLogger = requires(Spy<T>& spy, Logger&& logger) {
    spy.setLogger(std::forward<Logger>(logger));
};

template <Enable options>
struct EmptyLogger : Operations<options> {
    void operator ()(unsigned int) {}
};

}

void checkConceptPreservation() {
    using mpg::Enable;
    using mpg::Operations;

    using Semiregular = Operations<Enable{}>;
    using Copyable = Operations<Enable{ .default_constructor = false }>;
    using NoCopy = Operations<Enable{ .copy_constructor = false, .copy_assignment = false }>;
    using NoMove = Operations<Enable{ .move_constructor = false, .move_assignment = false }>;
    using NoCopyMoveConstructor = Operations<Enable{ .copy_constructor = false, .move_constructor = false }>;
    using NoCopyMoveAssignment = Operations<Enable{ .copy_assignment = false, .move_assignment = false }>;
    using BadDestructor = Operations<Enable{ .nothrow_destructor = false }>;

    static_assert(std::regular<Spy<int>>);

    static_assert(std::semiregular<Spy<Semiregular>>);
    static_assert(!std::regular<Spy<Semiregular>>);

    static_assert(std::copyable<Spy<Copyable>>);
    static_assert(!std::semiregular<Spy<Copyable>>);

    static_assert(std::movable<Spy<NoCopy>>);
    static_assert(!std::copyable<Spy<NoCopy>>);

    static_assert(std::semiregular<Spy<NoMove>>);
    static_assert(!std::regular<Spy<NoMove>>);

    static_assert(!std::movable<Spy<NoCopyMoveConstructor>>);
    static_assert(!std::movable<Spy<NoCopyMoveAssignment>>);
    static_assert(!std::movable<Spy<BadDestructor>>);
}

void checkLoggerSafety() {
    using mpg::Enable;
    using mpg::Operations;
    using mpg::detail::ProperSpyLogger;
    using mpg::detail::EmptyLogger;

    constexpr auto copyable_opt = Enable{ .default_constructor = false };
    constexpr auto move_only_opt = Enable{ .copy_constructor = false, .copy_assignment = false };
    constexpr auto bad_destructor_opt = Enable{ .nothrow_destructor = false };

    static_assert(ProperSpyLogger<EmptyLogger<copyable_opt>, Operations<copyable_opt>>);
    static_assert(!ProperSpyLogger<EmptyLogger<move_only_opt>, Operations<copyable_opt>>);
    static_assert(!ProperSpyLogger<EmptyLogger<bad_destructor_opt>, Operations<copyable_opt>>);

    static_assert(ProperSpyLogger<EmptyLogger<move_only_opt>, Operations<move_only_opt>>);
    static_assert(!ProperSpyLogger<EmptyLogger<copyable_opt>, Operations<move_only_opt>>);
    static_assert(!ProperSpyLogger<EmptyLogger<bad_destructor_opt>, Operations<move_only_opt>>);
}

int main() {
    checkConceptPreservation();
    checkLoggerSafety();
    return 0;
}
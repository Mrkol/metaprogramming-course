#pragma once

namespace mpg {

struct Enable {
    bool default_constructor = true;
    bool copy_constructor = true;
    bool move_constructor = true;
    bool copy_assignment = true;
    bool move_assignment = true;
    bool nothrow_destructor = true;
};

template <Enable options>
struct Operations {
    Operations() requires (options.default_constructor) = default;
    Operations(const Operations&) requires (options.copy_constructor) = default;
    Operations(Operations&&) noexcept requires (options.move_constructor) = default;
    Operations& operator =(const Operations&) requires (options.copy_assignment) = default;
    Operations& operator =(Operations&&) noexcept requires (options.move_assignment) = default;
    ~Operations() noexcept(options.nothrow_destructor) {}
};

}
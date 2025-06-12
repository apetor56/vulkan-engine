#pragma once

namespace ve::utils {

class NonMovable {
public:
    NonMovable()                                = default;
    NonMovable( NonMovable&& other )            = delete;
    NonMovable& operator=( NonMovable&& other ) = delete;
};

} // namespace ve::utils

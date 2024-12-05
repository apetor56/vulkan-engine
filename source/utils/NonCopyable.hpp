#pragma once

namespace utils {

class NonCopyable {
public:
    NonCopyable()                                      = default;
    NonCopyable( const NonCopyable& other )            = delete;
    NonCopyable& operator=( const NonCopyable& other ) = delete;
};

} // namespace utils

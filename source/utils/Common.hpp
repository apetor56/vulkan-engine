#pragma once

#include <ranges>

namespace utils {

constexpr auto size( const std::ranges::range auto& container ) {
    return static_cast< uint32_t >( std::size( container ) );
}

} // namespace utils
